/*
  Copyright (c) 2016-2020 Peter Antypas

  This file is part of the MAIANA™ transponder firmware.

  The firmware is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>
 */


#include "TXScheduler.hpp"
#include "EventQueue.hpp"
#include "AISMessages.hpp"
#include "TXPacket.hpp"
#include "config.h"
#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>
#include "RadioManager.hpp"
#include "ChannelManager.hpp"
#include "TXErrors.h"
#include "printf_serial.h"
#include "bsp.hpp"

using namespace std;

TXScheduler &TXScheduler::instance()
{
  static TXScheduler __instance;
  return __instance;
}

TXScheduler::TXScheduler ()
{
  EventQueue::instance().addObserver(this, GPS_FIX_EVENT | CLOCK_EVENT | INTERROGATION_EVENT);
  mPositionReportChannel = CH_87;
  mStaticDataChannel = CH_87;
  mUTC = 0;
  mAvgSpeed = 0.0;
  mLast18Time = 0;
  mLast24Time = 0;
}

void TXScheduler::init()
{
  if ( Configuration::instance().readStationData(mStationData) )
    {
      DBG("Successfully loaded Station Data \r\n");
    }
  else
    {
      DBG("Failed to read Station Data !!!\r\n");
    }

  bool cliBootMode = *(uint32_t*)BOOTMODE_ADDRESS == CLI_FLAG_MAGIC;
  if ( !cliBootMode )
    reportTXStatus();
}

TXScheduler::~TXScheduler ()
{
}

void TXScheduler::reportTXStatus()
{
  bool hwSwitchOff = bsp_is_tx_disabled();
  bool softSwitch = Configuration::instance().isTXEnabled();
  bool hasStation = Configuration::instance().isStationDataProvisioned();

  bool status = hwSwitchOff ? false : (softSwitch && hasStation);

  Event *e = EventPool::instance().newEvent(PROPR_NMEA_SENTENCE);
  if ( !e )
    return;

  sprintf(e->nmeaBuffer.sentence, "$PAITXCFG,%d,%d,%d,%d,%d*", bsp_is_tx_hardwired(), !hwSwitchOff, softSwitch, hasStation, status);
  Utils::completeNMEA(e->nmeaBuffer.sentence);
  EventQueue::instance().push(e);
}

bool TXScheduler::isTXAllowed()
{
  bool hwSwitchOff = bsp_is_tx_disabled();
  if ( hwSwitchOff )
    return false;

  bool softSwitch = Configuration::instance().isTXEnabled();
  bool hasStation = Configuration::instance().isStationDataProvisioned();

  return softSwitch && hasStation;
}

void TXScheduler::processEvent(const Event &e)
{
  switch(e.type)
  {
  case GPS_FIX_EVENT:
    {
      mLastGPSFix = e.gpsFix;


      // We do not schedule transmissions if the ChannelManager is not sure what channels are in use yet
      if ( !ChannelManager::instance().channelsDetermined() )
        return;

      // A class B transponder only transmits when its internal GPS is working, so we tie these to GPS updates.
      if ( !RadioManager::instance().initialized() || mUTC == 0 )
        return;

      if ( !isTXAllowed() )
        return;

      // Using a moving average of SOG to determine transmission rate
      static float alpha = 0.2;
      mAvgSpeed = mAvgSpeed * (1.0 - alpha) + mLastGPSFix.speed * alpha;

      if ( mUTC - mLast18Time > positionReportTimeInterval() )
        {
          queueMessage18(mPositionReportChannel);
          // Our next position report should be on the other channel
          mPositionReportChannel = RadioManager::instance().alternateChannel(mPositionReportChannel);
          mLast18Time = mUTC;
        }

      if ( mUTC - mLast24Time > MSG_24_TX_INTERVAL )
        {
          queueMessage24(mStaticDataChannel);
          // Our next static data report should be on the other channel
          mStaticDataChannel = RadioManager::instance().alternateChannel(mStaticDataChannel);
          mLast24Time = mUTC;
        }

      break;
    }
  case CLOCK_EVENT:
    {
      // This is reliable and independent of GPS update frequency which could change to something other than 1Hz
      if ( mUTC == 0 )
        {
          // Don't start transmitting right away
          mLast18Time = e.clock.utc - MAX_MSG_18_TX_INTERVAL/2;
          mLast24Time = e.clock.utc - MSG_24_TX_INTERVAL/2;
        }

      mUTC = e.clock.utc;

      break;
    }

    break;
  case INTERROGATION_EVENT:
    if ( e.interrogation.messageType == 18 )
      queueMessage18(e.interrogation.channel);

    if ( e.interrogation.messageType == 24 )
      queueMessage24(e.interrogation.channel);
    break;
  default:
    break;
  }

}

void TXScheduler::sendNMEASentence(const char *sentence)
{
  Event *e = EventPool::instance().newEvent(PROPR_NMEA_SENTENCE);
  if ( !e )
    return;

  strlcpy(e->nmeaBuffer.sentence, sentence, sizeof e->nmeaBuffer.sentence);
  Utils::completeNMEA(e->nmeaBuffer.sentence);
  EventQueue::instance().push(e);
}

void TXScheduler::queueMessage18(VHFChannel channel)
{
#if REPORT_TX_SCHEDULING
  char sentence[48];
#endif

  // If we don't have valid station data we don't do anything
  if ( mStationData.magic != STATION_DATA_MAGIC )
    return;

  TXPacket *p1 = TXPacketPool::instance().newTXPacket(channel);
  if ( !p1 )
    {
#if REPORT_TX_SCHEDULING
      sprintf(sentence, "$PAISCHTX,18,%d*", TX_ALLOC_ERROR);
      sendNMEASentence(sentence);
#endif
      return;
    }

  AISMessage18 msg;
  msg.latitude    = mLastGPSFix.lat;
  msg.longitude   = mLastGPSFix.lng;
  msg.sog         = mLastGPSFix.speed;
  msg.cog         = mLastGPSFix.cog;
  msg.utc         = mLastGPSFix.utc;
  msg.encode (mStationData, *p1);

  RadioManager::instance ().scheduleTransmission (p1);
}

void TXScheduler::queueMessage24(VHFChannel channel)
{
#if REPORT_TX_SCHEDULING
  char sentence[48];
#endif

  // If we don't have valid station data we don't do anything
  if ( mStationData.magic != STATION_DATA_MAGIC )
    return;

  TXPacket *p2 = TXPacketPool::instance().newTXPacket(channel);
  if ( !p2 )
    {
#if REPORT_TX_SCHEDULING
      sprintf(sentence, "$PAISCHTX,24A,%d*", TX_ALLOC_ERROR);
      sendNMEASentence(sentence);
#endif
      return;
    }

  AISMessage24A msg2;
  msg2.encode(mStationData, *p2);

  RadioManager::instance().scheduleTransmission(p2);

  TXPacket *p3 = TXPacketPool::instance().newTXPacket(channel);
  if ( !p3 )
    {
#if REPORT_TX_SCHEDULING
      sprintf(sentence, "$PAISCHTX,24B,%d*", TX_ALLOC_ERROR);
      sendNMEASentence(sentence);
#endif
      return;
    }

  AISMessage24B msg3;
  msg3.encode(mStationData, *p3);
  RadioManager::instance().scheduleTransmission(p3);

}

time_t TXScheduler::positionReportTimeInterval()
{
  // As a class B "CS" transponder, we transmit at a rate based on our speed (2 knots is the threshold)
  if ( mAvgSpeed < 2.0 )
    return MAX_MSG_18_TX_INTERVAL;

  return MIN_MSG_18_TX_INTERVAL;
}

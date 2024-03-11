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


#include "RadioManager.hpp"
#include "NoiseFloorDetector.hpp"
#include "bsp.hpp"
#include "TXErrors.h"

void rxClockCB();
void trxClockCB();


RadioManager &RadioManager::instance()
{
  static RadioManager __instance;
  return __instance;
}

RadioManager::RadioManager()
: mTXQueue(4)
{
  mTransceiverIC = NULL;
  mReceiverIC = NULL;
  mInitializing = true;
  mUTC = 0;
  EventQueue::instance().addObserver(this, CLOCK_EVENT);
}

bool RadioManager::initialized()
{
  return !mInitializing;
}

void RadioManager::init()
{
  NoiseFloorDetector::instance();
  mTransceiverIC = new Transceiver(SDN1_PORT, SDN1_PIN,
      CS1_PORT, CS1_PIN,
      TRX_IC_DATA_PORT, TRX_IC_DATA_PIN,
      TRX_IC_CLK_PORT, TRX_IC_CLK_PIN, 0);

  if ( mTransceiverIC->isResponsive() )
    {
      mTransceiverIC->init();
    }
  else
    {
      reportError(1);
    }

  mReceiverIC = new Receiver(SDN2_PORT, SDN2_PIN,
      CS2_PORT, CS2_PIN,
      RX_IC_DATA_PORT, RX_IC_DATA_PIN,
      RX_IC_CLK_PORT, RX_IC_CLK_PIN, 1);
  if ( mReceiverIC->isResponsive() )
    {
      mReceiverIC->init();
    }
  else
    {
      reportError(2);
    }

  if ( mReceiverIC->isResponsive() && mTransceiverIC->isResponsive() )
    mInitializing = false;
}

void RadioManager::setXOTrimValue(uint8_t value)
{
  if ( mTransceiverIC )
    mTransceiverIC->setXOTrimValue(value);

  if ( mReceiverIC )
    mReceiverIC->setXOTrimValue(value);
}

void RadioManager::transmitCW(VHFChannel channel)
{
  if ( mTransceiverIC )
    mTransceiverIC->transmitCW(channel);
}

void RadioManager::stopTX()
{
  if ( mTransceiverIC )
    mTransceiverIC->startReceiving(CH_87, true);
}

void RadioManager::start()
{
  configureInterrupts();
  if ( mTransceiverIC )
    mTransceiverIC->startReceiving(CH_87, true);

  if ( mReceiverIC )
    mReceiverIC->startReceiving(CH_88, true);

  GPS::instance().setDelegate(this);
}

void RadioManager::stop()
{
  // TODO: Implement this
}

void RadioManager::reportError(int chipId)
{
  Event *e = EventPool::instance().newEvent(PROPR_NMEA_SENTENCE);
  if ( !e )
    return;

  sprintf(e->nmeaBuffer.sentence, "$PAIERR,RFIC,%d*", chipId);
  Utils::completeNMEA(e->nmeaBuffer.sentence);
  EventQueue::instance().push(e);
}

void RadioManager::configureInterrupts()
{
  bsp_set_trx_clk_callback(trxClockCB);
  bsp_set_rx_clk_callback(rxClockCB);
}

void RadioManager::processEvent(const Event &e)
{
  mUTC = e.clock.utc;

  if ( mStartTime == 0 )
    {
      mStartTime = mUTC;
    }

  if ( !mTXQueue.empty() && mTransceiverIC->assignedTXPacket() == NULL )
    {
      // There is no current TX operation pending, so we assign one
      TXPacket *packet = NULL;
      mTXQueue.pop(packet);
      ASSERT(packet);

      VHFChannel txChannel = packet->channel();

      // Do we need to swap channels?
      if ( txChannel != mTransceiverIC->channel() )
        {
          // The receiver needs to be explicitly told to switch channels
          if ( mReceiverIC )
            mReceiverIC->switchToChannel(alternateChannel(txChannel));
        }

      // The transceiver will switch channel if the packet channel is different
      mTransceiverIC->assignTXPacket(packet);
    }
  else if ( mUTC - mStartTime >= 3600 )
    {
      /**
       * For some reason, RX performance seems to go downhill after many hours
       * of continuous operation, so the easiest remedy is to reboot every hour.
       * The impact of this is pretty small.
       */

      bsp_reboot();
    }

}

VHFChannel RadioManager::alternateChannel(VHFChannel channel)
{
  // TODO: Delegate this to the ChannelManager
  return channel == CH_88 ? CH_87 : CH_88;
}

void RadioManager::onBitClock(uint8_t ic)
{
  if ( mInitializing )
    return;

  if ( ic == 1 && mTransceiverIC )
    mTransceiverIC->onBitClock();
  else if ( mReceiverIC )
    mReceiverIC->onBitClock();
}

void RadioManager::timeSlotStarted(uint32_t slotNumber)
{
  if ( mInitializing )
    return;

  mTransceiverIC->timeSlotStarted(slotNumber);
  mReceiverIC->timeSlotStarted(slotNumber);
}

void RadioManager::scheduleTransmission(TXPacket *packet)
{
#if REPORT_TX_SCHEDULING
  Event *e = EventPool::instance().newEvent(PROPR_NMEA_SENTENCE);
#endif
  if ( !mTXQueue.push(packet) )
    {
#if REPORT_TX_SCHEDULING
      if ( e )
        {
          sprintf(e->nmeaBuffer.sentence, "$PAISCHTX,%s,%d*", packet->messageType(), TX_QUEUE_FULL);
          Utils::completeNMEA(e->nmeaBuffer.sentence);
          EventQueue::instance().push(e);
        }
#endif
      TXPacketPool::instance().deleteTXPacket(packet);
    }
#if REPORT_TX_SCHEDULING
  else
    {
      if ( e )
        {
          sprintf(e->nmeaBuffer.sentence, "$PAISCHTX,%s,%d*", packet->messageType(), TX_NO_ERROR);
          Utils::completeNMEA(e->nmeaBuffer.sentence);
          EventQueue::instance().push(e);
        }
    }
#endif
}

void RadioManager::sendTestPacketNow(TXPacket *packet)
{
  if ( mTransceiverIC )
    {
      mTransceiverIC->assignTXPacket(packet);
    }
}

void rxClockCB()
{
  RadioManager::instance().onBitClock(2);
}

void trxClockCB()
{
  RadioManager::instance().onBitClock(1);
}




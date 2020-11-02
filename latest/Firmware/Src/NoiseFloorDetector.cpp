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


#include "NoiseFloorDetector.hpp"
#include "EventQueue.hpp"
#include "AISChannels.h"


#define WINDOW_SIZE 10

NoiseFloorDetector &NoiseFloorDetector::instance()
{
  static NoiseFloorDetector __instance;
  return __instance;
}

NoiseFloorDetector::NoiseFloorDetector()
: mTicks(0xffffffff)
{
  //mChannelASamples.reserve(WINDOW_SIZE);
  //mChannelBSamples.reserve(WINDOW_SIZE);
  mChannelACurrent = 0xff;
  mChannelBCurrent = 0xff;

  mAFloor = 0xff;
  mBFloor = 0xff;
  EventQueue::instance().addObserver(this, CLOCK_EVENT);
}

void NoiseFloorDetector::report(char channel, uint8_t rssi)
{
  if ( rssi < 0x20 ) // Not realistic, likely a bug
    return;

#if 0
  processSample(channel == 'A' ? mChannelASamples : mChannelBSamples, rssi);

  if ( channel == 'A' )
    mChannelACurrent = medianValue(mChannelASamples);
  else
    mChannelBCurrent = medianValue(mChannelBSamples);
#endif

  if ( channel == 'A' )
    {
      mAFloor = min(mAFloor, rssi);
    }
  else
    {
      mBFloor = min(mBFloor, rssi);
    }
}

uint8_t NoiseFloorDetector::getNoiseFloor(char channel)
{
  return channel == 'A' ? mChannelACurrent : mChannelBCurrent;
}

void NoiseFloorDetector::processEvent(const Event &e)
{
  switch(e.type)
  {
  case CLOCK_EVENT:
    if ( mTicks == 0xffffffff )
      {
        mTicks = 0;
      }
    else
      {
        ++mTicks;
      }

    if ( mTicks == 10 )
      {
        //DBG("Event pool utilization = %d, max = %d\r\n", EventPool::instance().utilization(), EventPool::instance().maxUtilization());
        mChannelACurrent = mAFloor;
        mChannelBCurrent = mBFloor;
        dump();
        reset();
        mTicks = 0;
      }
    break;
#if 0
  case RSSI_SAMPLE_EVENT:
    {
      report(e.rssiSample.channel, e.rssiSample.rssi);
      uint8_t rssi = getNoiseFloor(e.rssiSample.channel);
      RadioManager::instance().noiseFloorUpdated(e.rssiSample.channel, rssi);
    }
    break;
#endif
  default:
    break;
  }
}

#if 0
void NoiseFloorDetector::processSample(ChannelReadings &window, uint8_t rssi)
{
  while ( window.size() >= WINDOW_SIZE )
    window.pop_back();

  if ( window.empty() )
    {
      window.push_back(rssi);
      return;
    }

  // Insert the reading at the start if it qualifies
  for ( ChannelReadings::iterator i = window.begin(); i != window.end(); ++i )
    {
      if ( rssi <= *i )
        {
          window.insert(i, rssi);
          break;
        }
    }

}

uint8_t NoiseFloorDetector::medianValue(ChannelReadings &window)
{
  if ( window.size() < WINDOW_SIZE )
    return 0xff;

  return window[window.size()/2];
}

#endif

void NoiseFloorDetector::dump()
{
  Event e(PROPR_NMEA_SENTENCE);

  sprintf(e.nmeaBuffer.sentence, "$PAINF,A,0x%.2x*", mChannelACurrent);
  Utils::completeNMEA(e.nmeaBuffer.sentence);
  EventQueue::instance().push(e);

  sprintf(e.nmeaBuffer.sentence, "$PAINF,B,0x%.2x*", mChannelBCurrent);
  Utils::completeNMEA(e.nmeaBuffer.sentence);
  EventQueue::instance().push(e);
}

void NoiseFloorDetector::reset()
{
  //mChannelASamples.clear();
  //mChannelBSamples.clear();
  mAFloor = 0xff;
  mBFloor = 0xff;
}




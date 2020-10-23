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
#include <algorithm>
#include "AISChannels.h"
#include "_assert.h"
#include "printf_serial.h"
#include "RadioManager.hpp"


#define WINDOW_SIZE 10


NoiseFloorDetector &NoiseFloorDetector::instance()
{
  static NoiseFloorDetector __instance;
  return __instance;
}

NoiseFloorDetector::NoiseFloorDetector()
: mTicks(0xffffffff)
{
  EventQueue::instance().addObserver(this, CLOCK_EVENT|RSSI_SAMPLE_EVENT);
}

void NoiseFloorDetector::report(VHFChannel channel, uint8_t rssi)
{
  if ( rssi < 0x20 ) // Not realistic, likely a bug
    return;

  if ( mData.find(channel) == mData.end() )
    {
      ChannelReadings r;
      r.reserve(WINDOW_SIZE*2);
      mData[channel] = r;
    }

  ChannelReadings &window = mData[channel];
  processSample(window, rssi);
}

uint8_t NoiseFloorDetector::getNoiseFloor(VHFChannel channel)
{
  if ( mData.find(channel) == mData.end() )
    return 0xff;

  return medianValue(mData[channel]);
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

    if ( mTicks == 30 )
      {
        //DBG("Event pool utilization = %d, max = %d\r\n", EventPool::instance().utilization(), EventPool::instance().maxUtilization());
        //dump();
        reset();
        mTicks = 0;
      }
    break;
  case RSSI_SAMPLE_EVENT:
    {
      report(e.rssiSample.channel, e.rssiSample.rssi);
      uint8_t rssi = getNoiseFloor(e.rssiSample.channel);
      RadioManager::instance().noiseFloorUpdated(e.rssiSample.channel, rssi);
    }
    break;
  default:
    break;
  }
}

void NoiseFloorDetector::processSample(ChannelReadings &window, uint8_t rssi)
{
  while ( window.size() >= WINDOW_SIZE )
    window.pop_back();

  if ( window.empty() )
    {
      Reading r;
      r.reading = rssi;
      window.push_back(r);
      return;
    }

  // Insert the reading at the start if it qualifies
  for ( auto i = window.begin(); i != window.end(); ++i )
    {
      if ( rssi <= i->reading )
        {
          Reading r;
          r.reading = rssi;
          window.insert(i, r);
          break;
        }
    }
}

uint8_t NoiseFloorDetector::medianValue(ChannelReadings &window)
{
  if ( window.empty() )
    return 0xff;

  return window[window.size()/2].reading;
}

#if 0

void NoiseFloorDetector::dump()
{
  for ( ChannelData::iterator cIt = mData.begin(); cIt != mData.end(); ++cIt )
    {
      //uint8_t value = medianValue(cIt->second);
      //DBG("[Channel %d noise floor: 0x%.2x]\r\n", AIS_CHANNELS[cIt->first].itu, value);
    }
}
#endif

void NoiseFloorDetector::reset()
{
  mData.clear();
}




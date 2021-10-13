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
#include <stdio.h>
#include <bsp/bsp.hpp>

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
#if BOARD_REV == 61 or BOARD_REV == 52
  if ( rssi < 0x12 ) // Not realistic, likely a bug
#elif BOARD_REV == 109
    if ( rssi < 0x24 )
#else
  if ( rssi < 0x10 ) // Not realistic, likely a bug
#endif
    return;

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

    if ( mTicks == 30 )
      {
        //DBG("Event pool utilization = %d, max = %d\r\n", EventPool::instance().utilization(), EventPool::instance().maxUtilization());
        recalculate();
        dump();
        mTicks = 0;
      }
    break;
  default:
    break;
  }
}

void NoiseFloorDetector::dump()
{
  Event *e = EventPool::instance().newEvent(PROPR_NMEA_SENTENCE);
  if ( !e )
    return;

  sprintf(e->nmeaBuffer.sentence, "$PAINF,A,0x%.2x*", mChannelACurrent);
  Utils::completeNMEA(e->nmeaBuffer.sentence);
  EventQueue::instance().push(e);

  e = EventPool::instance().newEvent(PROPR_NMEA_SENTENCE);
  if ( !e )
    return;

  sprintf(e->nmeaBuffer.sentence, "$PAINF,B,0x%.2x*", mChannelBCurrent);
  Utils::completeNMEA(e->nmeaBuffer.sentence);
  EventQueue::instance().push(e);
}

void NoiseFloorDetector::recalculate()
{
  if ( mChannelACurrent == 0xff || mChannelBCurrent == 0xff )
    {
      mChannelACurrent = mAFloor;
      mChannelBCurrent = mBFloor;
    }
  else
    {
      mChannelACurrent = (mChannelACurrent + mAFloor) / 2;
      mChannelBCurrent = (mChannelBCurrent + mBFloor) / 2;
    }

  mAFloor = 0xff;
  mBFloor = 0xff;
}




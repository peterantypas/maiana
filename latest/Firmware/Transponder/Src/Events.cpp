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


#include "Events.hpp"
#include "printf_serial.h"


///////////////////////////////////////////////////////////////////////////////
//
// Event
//
///////////////////////////////////////////////////////////////////////////////

Event::Event()
  : type(UNKNOWN_EVENT), flags(0), rxPacket(nullptr)
{
}

void Event::reset()
{
  if ( rxPacket )
    {
      ASSERT_VALID_PTR(rxPacket);
      EventPool::instance().releaseRXPacket(rxPacket);
    }

  rxPacket = nullptr;

  type = UNKNOWN_EVENT;
}

///////////////////////////////////////////////////////////////////////////////
//
// EventPool
//
///////////////////////////////////////////////////////////////////////////////

EventPool &EventPool::instance()
{
  static EventPool __instance;
  return __instance;
}


EventPool::EventPool()
  : mISRPool(25), mThreadPool(10), mRXPool(20)
{

}

void EventPool::init()
{
}

Event *EventPool::newEvent(EventType type)
{
  Event *result = nullptr;
  if ( Utils::inISR() )
    {
      result = mISRPool.get();
      if ( result )
        {
          result->type = type;
          result->flags = 1;
          ASSERT(!result->rxPacket);
        }
    }
  else
    {
      result = mThreadPool.get();
      if ( result )
        {
          result->type = type;
          result->flags = 0;
          ASSERT(!result->rxPacket);
        }
    }


  //if ( !result)
    //printf2_now("\r\n[DEBUG]newEvent(0x%.8x) failed\r\n", type);

  if ( result == nullptr )
    return result;

  ASSERT_VALID_PTR(result);
  return result;
}

void EventPool::deleteEvent(Event *event)
{
  ASSERT_VALID_PTR(event);
  event->reset();
  if ( event->flags )
    mISRPool.put(event);
  else
    mThreadPool.put(event);
}

uint32_t EventPool::maxUtilization()
{
  return std::max(mISRPool.maxUtilization(), mThreadPool.maxUtilization());
}

uint32_t EventPool::utilization()
{
  return std::max(mISRPool.utilization(), mThreadPool.utilization());
}

RXPacket *EventPool::newRXPacket()
{
  RXPacket *p = mRXPool.get();
  ASSERT_VALID_PTR(p);
  return p;
}

void EventPool::releaseRXPacket(RXPacket *packet)
{
  ASSERT_VALID_PTR(packet);
  mRXPool.put(packet);
}


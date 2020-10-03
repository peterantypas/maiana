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


#include "EventQueue.hpp"
#include <cassert>
#include <stm32l4xx.h>

#include "printf_serial.h"
#include "printf_serial.h"
#include "Utils.hpp"


EventQueue &EventQueue::instance()
{
  static EventQueue __instance;
  return __instance;
}

EventQueue::EventQueue()
{
  mISRQueue = new CircularQueue<Event*>(40);
  mThreadQueue = new CircularQueue<Event*>(40);
}

void EventQueue::init()
{
}

void EventQueue::push(Event *event)
{
  /*
   * For mISRQueue, interrupt context is the "producer", and the thread is the "consumer".
   * For mThreadQueue, the thread is both the producer and the consumer, so all access is serialized.
   */
  ASSERT(event);
  if ( Utils::inISR() )
    {
      if ( !mISRQueue->push(event) )
        {
          EventPool::instance().deleteEvent(event);
        }
    }
  else
    {
      if ( !mThreadQueue->push(event) )
        {
          EventPool::instance().deleteEvent(event);
        }
    }
}

void EventQueue::addObserver(EventConsumer *c, uint32_t eventMask)
{
  mConsumers[c] = eventMask;
}

void EventQueue::removeObserver(EventConsumer *c)
{
  map<EventConsumer*, uint32_t>::iterator i = mConsumers.find(c);
  if ( i == mConsumers.end() )
    return;

  mConsumers.erase(i);
}

void EventQueue::dispatch()
{
  //ASSERT(!Utils::inISR());

  Event *e = nullptr;

  // This is safe to do as interrupt context never pops this queue!
  while ( mISRQueue->pop(e) )
    {
      mThreadQueue->push(e);
    }

  if ( mThreadQueue->pop(e) )
    {
      for ( map<EventConsumer*, uint32_t>::iterator c = mConsumers.begin(); c != mConsumers.end(); ++c )
        {
          if ( c->second & e->type )
            {
              c->first->processEvent(*e);
            }
        }

      EventPool::instance().deleteEvent(e);
    }
}


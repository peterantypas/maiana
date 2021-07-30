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
#include <bsp/bsp.hpp>


EventQueue &EventQueue::instance()
{
  static EventQueue __instance;
  return __instance;
}

EventQueue::EventQueue()
  : mISRQueue(25), mTaskQueue(10)
{
}

void EventQueue::init()
{
}

bool EventQueue::push(Event *e)
{
  if ( bsp_is_isr() )
    {
      if ( !mISRQueue.push(e) )
        {
          EventPool::instance().deleteEvent(e);
          return false;
        }
    }
  else
    {
      if ( !mTaskQueue.push(e) )
        {
          EventPool::instance().deleteEvent(e);
          return false;
        }
    }

  return true;
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
  Event *e = nullptr;

  while (mISRQueue.pop(e))
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

  while (mTaskQueue.pop(e))
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


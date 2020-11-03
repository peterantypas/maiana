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
//#include <stm32l4xx.h>

#include "printf_serial.h"
#include "printf_serial.h"
#include "Utils.hpp"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "bsp.hpp"

#define EVENT_QUEUE_SIZE  50

//static Event* __queue[EVENT_QUEUE_SIZE];

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
  //mQueueHandle = xQueueCreateStatic(EVENT_QUEUE_SIZE, sizeof(Event*), (uint8_t*)&__queue[0], &mQueue);
  //configASSERT(mQueueHandle);
}

void EventQueue::push(Event *e)
{
  //if ( xTaskGetSchedulerState() != taskSCHEDULER_RUNNING )
    //return;

  //BaseType_t xHighPriorityTaskWoken = pdTRUE;
  if ( Utils::inISR() )
    {
      //bsp_signal_high();
      //xQueueSendFromISR(mQueueHandle, &e, &xHighPriorityTaskWoken);
      //bsp_signal_low();
      if ( !mISRQueue.push(e) )
        EventPool::instance().deleteEvent(e);
#if 0
      if ( xHighPriorityTaskWoken )
        portYIELD_FROM_ISR(xHighPriorityTaskWoken);
#endif
    }
  else
    {
      //xQueueSend(mQueueHandle, &e, 0);
      if ( !mTaskQueue.push(e) )
        EventPool::instance().deleteEvent(e);
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
  Event *e = nullptr;

  //while ( xQueueReceive(mQueueHandle, &e, 10) == pdTRUE )
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


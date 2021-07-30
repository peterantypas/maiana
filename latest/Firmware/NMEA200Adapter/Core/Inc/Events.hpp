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


#ifndef EVENTS_HPP_
#define EVENTS_HPP_

#include "EventTypes.h"
#include <time.h>
#include <cstring>
#include <string>
#include "ObjectPool.hpp"

typedef struct {
  char sentence[120];
} NMEABuffer;

class Event
{
public:
  EventType type;
  uint32_t flags;

  Event();


  void reset();


  union {
    NMEABuffer nmeaBuffer;
  };
};


/*
 * Event consumer abstract definition.
 */

class EventConsumer
{
public:
  virtual ~EventConsumer() {}
  virtual void processEvent(const Event &event)=0;
};


class EventPool
{
public:
  static EventPool &instance();

  void init();
  Event *newEvent(EventType type);
  void deleteEvent(Event *event);
  uint32_t utilization();
  uint32_t maxUtilization();
private:
  EventPool();

private:
  ObjectPool<Event>     mISRPool;
  ObjectPool<Event>     mThreadPool;
};

#endif /* EVENTS_HPP_ */

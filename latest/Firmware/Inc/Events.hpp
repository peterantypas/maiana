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
#include "NMEASentence.hpp"
#include <string>
#include "RXPacket.hpp"
#include "ObjectPool.hpp"
#include "AISChannels.h"
//#include "RadioManager.hpp"

using namespace std;

typedef struct {
  char sentence[120];
} NMEABuffer;

typedef struct {
  time_t utc;
  float lat;
  float lng;
  float speed;
  float cog;
} GPSFix;

typedef struct {
  time_t utc;
} ClockTick;

typedef struct {
  char buffer[120];
} DebugMessage;

typedef enum {
  OP_GET,
  OP_SET
} Operation;

typedef struct {
  VHFChannel channel;
  uint8_t messageType;
} Interrogation;

typedef struct {
  VHFChannel channel;
  uint8_t rssi;
} RSSISample;


class Event
{
public:
  EventType type;
  uint32_t flags;

  Event();

#if 0
  Event(EventType t)
    : type(t), flags(0), rxPacket(nullptr)
  {
  }
#endif


  void reset();

  // This is an object, so it can't be a member of the union ...
  RXPacket *rxPacket;

  union {
    NMEABuffer nmeaBuffer;
    GPSFix gpsFix;
    DebugMessage debugMessage;
    ClockTick clock;
    Interrogation interrogation;
    RSSISample rssiSample;
    DebugMessage command;
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
  RXPacket *newRXPacket();
  void releaseRXPacket(RXPacket *);
private:
  EventPool();

private:
  ObjectPool<Event>     mISRPool;
  ObjectPool<Event>     mThreadPool;
  ObjectPool<RXPacket>  mRXPool;
};

#endif /* EVENTS_HPP_ */

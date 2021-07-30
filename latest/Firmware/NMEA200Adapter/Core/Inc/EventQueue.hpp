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


#ifndef EVENTQUEUE_HPP_
#define EVENTQUEUE_HPP_

#include <map>
#include "CircularQueue.hpp"
#include "Events.hpp"


using namespace std;

class EventQueue
{
public:
  static EventQueue &instance();

  void init();

  /*
   * Consumer registration
   */
  void addObserver(EventConsumer *c, uint32_t eventMask);

  /*
   * Consumer de-registration
   */
  void removeObserver(EventConsumer *c);

  /*
   * We push events here to be processed by the dispatching task
   */
  bool push(Event *event);

  /*
   * This method must be called repeatedly by an RTOS task or main() (never an ISR)
   */
  void dispatch();
private:
  EventQueue();
  CircularQueue<Event*> mISRQueue;
  CircularQueue<Event*> mTaskQueue;
  map<EventConsumer *, uint32_t> mConsumers;
};

#endif /* EVENTQUEUE_HPP_ */

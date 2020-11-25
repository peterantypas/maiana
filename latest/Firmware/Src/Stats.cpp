/*
 * Stats.cpp
 *
 *  Created on: Nov 4, 2020
 *      Author: peter
 */


#include "Stats.hpp"
#include "Utils.hpp"
#include "EventQueue.hpp"
#include <stdio.h>

static int count = 0;

Stats &Stats::instance()
{
  static Stats __instance;
  return __instance;
}

Stats::Stats()
{
  EventQueue::instance().addObserver(this, CLOCK_EVENT);
}

void Stats::init()
{

}

void Stats::processEvent(const Event &e)
{
  ++count;
  if ( count % 60 == 0 )
    {
      char buff[32];
      sprintf(buff, "$PAISTC,%d,%d,%d*", eventQueuePopFailures, eventQueuePushFailures, rxPacketPoolPopFailures);
      Utils::completeNMEA(buff);

      printf_serial(buff);
      count = 1;
    }
}






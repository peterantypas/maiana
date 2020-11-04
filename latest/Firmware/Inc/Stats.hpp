/*
 * Stats.hpp
 *
 *  Created on: Nov 4, 2020
 *      Author: peter
 */

#ifndef INC_STATS_HPP_
#define INC_STATS_HPP_

#include <stdint.h>
#include "EventQueue.hpp"

class Stats : public EventConsumer
{
public:
  static Stats &instance();
  void init();

  void processEvent(const Event &e);

private:
  Stats();
public:
  int eventQueuePopFailures       = 0;
  int eventQueuePushFailures      = 0;
  int rxPacketPoolPopFailures     = 0;
};



#endif /* INC_STATS_HPP_ */

/*
 * DebugMsgHandler.hpp
 *
 *  Created on: Jul 8, 2021
 *      Author: peter
 */

#ifndef INC_DEBUGMSGHANDLER_HPP_
#define INC_DEBUGMSGHANDLER_HPP_

#include <NMEA2000.h>

class DebugMsgHandler: public tNMEA2000::tMsgHandler
{
public:
  void HandleMsg(const tN2kMsg &N2kMsg);
};


#endif /* INC_DEBUGMSGHANDLER_HPP_ */

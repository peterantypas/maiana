/*
 * DebugMsgHandler.cpp
 *
 *  Created on: Jul 8, 2021
 *      Author: peter
 */

#include "DebugMsgHandler.hpp"
#include <stdio.h>

void DebugMsgHandler::HandleMsg(const tN2kMsg &msg)
{
  printf("Got PGN %d:\r\n", msg.PGN);

  for ( int i = 0; i < msg.DataLen; ++i )
    {
      printf("%.2x ", msg.Data[i]);
    }

  printf("\r\n\r\n");
}





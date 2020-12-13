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


#include "DataTerminal.hpp"
#include "Events.hpp"
#include "Utils.hpp"
#include "bsp.hpp"

static char __rxbuff[80];
static uint8_t __rxpos = 0;

void termInputCB(char c);

DataTerminal &DataTerminal::instance()
{
  static DataTerminal __instance;
  return __instance;
}

void DataTerminal::init()
{
  bsp_set_terminal_input_callback(termInputCB);
}

DataTerminal::DataTerminal()
: mCmdBuffPos(0)
{
  mCmdTokens.reserve(5);
  EventQueue::instance().addObserver(this, DEBUG_EVENT|PROPR_NMEA_SENTENCE);
}

void DataTerminal::processEvent(const Event &e)
{
  switch (e.type) {
  case DEBUG_EVENT:
#ifdef MULTIPLEXED_OUTPUT
    write("DEBUG", e.debugMessage.buffer);
#endif
    break;
  case PROPR_NMEA_SENTENCE:
#ifdef MULTIPLEXED_OUTPUT
    write("NMEA", e.nmeaBuffer.sentence);
#else
    write(e.nmeaBuffer.sentence);
#endif
    break;
  default:
    break;
  }
}

#ifdef MULTIPLEXED_OUTPUT

void DataTerminal::write(const char *cls, const char* s)
{
  bsp_write_char('[');
  bsp_write_string(cls);
  bsp_write_char(']');
  bsp_write_string(s);
}

#else

void DataTerminal::write(const char* s)
{
  bsp_write_string(s);
}
#endif



void DataTerminal::_write(const char *s)
{
#ifdef MULTIPLEXED_OUTPUT
  write("", s);
#else
  write(s);
#endif
}


void termInputCB(char c)
{
  if ( c == '\r' )
    {
      return;
    }

  __rxbuff[__rxpos++] = c;
  if ( __rxpos >= sizeof __rxbuff )
    {
      __rxpos = 0;
    }
  else if ( c == '\n' )
    {
      __rxbuff[__rxpos++] = 0;
      Event *e = EventPool::instance().newEvent(COMMAND_EVENT);
      strlcpy(e->command.buffer, __rxbuff, sizeof e->command.buffer);
      EventQueue::instance().push(e);
      __rxpos = 0;
    }
}






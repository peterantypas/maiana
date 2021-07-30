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


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "bsp.hpp"
#include "config.h"
#include "Events.hpp"
#include "Utils.hpp"
#include "EventQueue.hpp"
#include "DataTerminal.hpp"

static char __buffer[128];

void printf_null(const char *format, ...)
{

}

void printf_serial_now(const char *format, ...)
{
  va_list list;
  va_start(list, format);
  vsnprintf(__buffer, sizeof __buffer, format, list);
  va_end(list);

  bsp_write_string(__buffer);
}


void printf_serial(const char *format, ...)
{
  if ( Utils::inISR() )
    {
      Event *e = EventPool::instance().newEvent(DEBUG_EVENT);
      if ( e )
        {
          va_list list;
          va_start(list, format);
          vsnprintf(e->debugMessage.buffer, sizeof e->debugMessage, format, list);
          va_end(list);

          EventQueue::instance().push(e);
        }
    }
  else
    {
      va_list list;
      va_start(list, format);
      vsnprintf(__buffer, sizeof __buffer, format, list);
      va_end(list);
#ifdef MULTIPLEXED_OUTPUT
      DataTerminal::instance().write("DEBUG", __buffer);
#else
      bsp_write_string(__buffer);
#endif
    }
}



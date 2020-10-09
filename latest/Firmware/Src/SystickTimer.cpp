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


#include "SystickTimer.hpp"
#include "EventQueue.hpp"
#include "printf_serial.h"

SystickTimer &SystickTimer::instance()
{
  static SystickTimer __instance;
  return __instance;
}

SystickTimer::SystickTimer()
: mTickCounter(0), mSecondCounter(0)
{

}

void SystickTimer::onTick()
{
  ++mTickCounter;
  if ( mTickCounter == 1000 )
    {
      mTickCounter = 0;
      Event e(ONE_SEC_TIMER_EVENT);
      EventQueue::instance().push(e);
      ++mSecondCounter;
    }

  if ( mSecondCounter == 60 )
    {
      mSecondCounter = 0;
      //DBG("One minute mark\r\n");
      Event e(ONE_MIN_TIMER_EVENT);
      EventQueue::instance().push(e);
    }
}

extern "C"
{
  void HAL_SYSTICK_Callback()
  {
    //SystickTimer::instance().onTick();
  }
}



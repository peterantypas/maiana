/*
  Copyright (c) 2016-2021 Peter Antypas

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
#ifndef INC_LEDMANAGER_HPP_
#define INC_LEDMANAGER_HPP_


class LEDManager
{
public:
  static LEDManager &instance();

  void init();

  // This is a 1ms tick callback
  void onTick();

private:
  LEDManager();
  ///volatile bool mForceTXLedOff = false;
};


#endif /* INC_LEDMANAGER_HPP_ */

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


#ifndef PRINTF2_H_
#define PRINTF2_H_

#include "config.h"


void printf_null(const char *format, ...);

// printf2 automatically determines ISR vs thread mode and schedules or prints immediately depending
void printf_serial(const char *format, ...);

// printf2_now outputs to UART immediately, disregarding ISR vs thread mode
void printf_serial_now(const char *format, ...);

#endif /* PRINTF2_H_ */

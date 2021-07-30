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

#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>
#include <vector>
#include <inttypes.h>

using namespace std;

class Utils
{
public:
  //static void delay(uint32_t ticks);
  static void makeLowercase(std::string &);
  static void makeUppercase(std::string &);
  static void trim(std::string &s);
  static void tokenize(const string &str, char delim, vector<string> &result);

  static int toInt(const std::string &);
  static uint16_t crc16(uint8_t* data, uint16_t len);
  static uint16_t reverseBits(uint16_t data);

  // NMEA-specific
  static float latitudeFromNMEA(const string &decimal, const string &hemisphere);
  static float longitudeFromNMEA(const string &decimal, const string &hemisphere);
  static uint32_t coordinateToUINT32(double value);
  static float coordinateFromUINT32(uint32_t aisCoordinate, uint8_t numBits);

  // ARM-specific utilities
  static bool inISR();
  static void completeNMEA(char *buff);

};

#endif
/* 
 Local Variables: ***
 mode: c++ ***
 End: ***
 */

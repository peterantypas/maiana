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

#include "Utils.hpp"

#include <cstring>
#include <algorithm>
#include <cstdio>
//#include <sstream>
#include <vector>
#include <limits>
#include <cmath>
#include <errno.h>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <bsp/bsp.hpp>




using namespace std;

void Utils::makeLowercase(string &s)
{
  std::transform(s.begin(), s.end(), s.begin(), static_cast<int (*)(int)>(tolower));
}

void Utils::makeUppercase(string &s)
{
  std::transform(s.begin(), s.end(), s.begin(), static_cast<int (*)(int)>(toupper));
}

void Utils::trim(string &str)
{
  static string whitespace(" \t\f\v\n\r");
  string::size_type pos = str.find_last_not_of(whitespace);
  if ( pos != string::npos )
    {
      str.erase(pos + 1);
      pos = str.find_first_not_of(whitespace);
      if ( pos != string::npos )
        str.erase(0, pos);
    }
  else
    str.erase(str.begin(), str.end());
}

int Utils::toInt(const std::string &s)
{
  if ( s.empty() )
    return 0;
  return atoi(s.c_str());
}


float Utils::latitudeFromNMEA(const string &decimal, const string &hemisphere)
{
  // Latitude always starts with 4 integers: 2 for degrees, 2 for minutes . N decimal minutes
  string degStr = decimal.substr(0, 2);
  string decStr = decimal.substr(2);

  float deg = atof(degStr.c_str());
  float min = atof(decStr.c_str());
  return (hemisphere == "N" ? 1 : -1) * (deg + min / 60.0f);
}

float Utils::longitudeFromNMEA(const string &decimal, const string &hemisphere)
{
  // Longitude always starts with 5 integers: 3 for degrees, 2 for minutes . N decimal minutes
  string degStr = decimal.substr(0, 3);
  string decStr = decimal.substr(3);

  float deg = atof(degStr.c_str());
  float min = atof(decStr.c_str());
  return (hemisphere == "E" ? 1 : -1) * (deg + min / 60.0f);
}

uint16_t Utils::reverseBits(uint16_t crc)
{
  uint16_t result = 0;
  for ( size_t i = 0; i < 16; ++i )
    {
      result <<= 1;
      result |= ((crc & (1 << i)) >> i);
    }

  return result;
}


void Utils::tokenize(const string &str, char delim, vector<string> &result)
{
  if ( str.empty() )
    return;

  string token;
  result.clear();
  for ( unsigned i = 0; i < str.length(); ++i )
    {
      if ( str[i] == delim )
        {
          result.push_back(token);
          token.erase();
        }
      else
        token += str[i];
    }

  if ( !token.empty() )
    result.push_back(token);
}


uint32_t Utils::coordinateToUINT32(double value)
{
  uint32_t val = fabs(value) * 600000;
  if ( value < 0.0 )
    val = ~val + 1;

  return val;
}

float Utils::coordinateFromUINT32(uint32_t value, uint8_t numBits)
{
  if ( value & (1 << (numBits - 1)) )
    {
      value = ~value;
      for ( int i = numBits - 1; i < 32; ++i )
        value &= ~(1 << i);

      return value / -600000.0f;
    }
  else
    {
      return value / 600000.0f;
    }
}

bool Utils::inISR()
{
  return bsp_is_isr();
}

void Utils::completeNMEA(char *buff)
{
  uint8_t p = 1;
  uint8_t crc = buff[p++];
  while ( buff[p] != '*' )
    crc ^= buff[p++];

  sprintf(buff+p+1, "%.2X\r\n", crc);
}


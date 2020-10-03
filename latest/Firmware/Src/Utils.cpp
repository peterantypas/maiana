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

// TODO: Get rid of this dependency and delegate inISR() to the BSP layer instead
#include "stm32l4xx.h"


static const uint16_t CRC16_XMODEM_TABLE[] ={
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108,
    0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231,
    0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339,
    0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462,
    0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a,
    0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d, 0x3653,
    0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b,
    0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4,
    0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 0xc9cc,
    0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5,
    0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd,
    0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 0x6ca6,
    0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae,
    0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97,
    0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 0xff9f,
    0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78, 0x9188,
    0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080,
    0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067, 0x83b9,
    0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1,
    0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea,
    0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2,
    0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xa7db,
    0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3,
    0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634, 0xd94c,
    0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844,
    0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d,
    0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75,
    0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e,
    0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26,
    0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1, 0xef1f,
    0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17,
    0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};


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

uint16_t Utils::crc16(uint8_t *bytes, uint16_t length)
{
  uint16_t crc = 0xffff;
  for ( uint16_t b = 0; b < length; ++b )
    {
      crc = ((crc << 8) & 0xff00) ^ CRC16_XMODEM_TABLE[((crc >> 8) & 0xff) ^ bytes[b]];
    }
  return Utils::reverseBits(~crc);
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
  return __get_IPSR();
}

void Utils::completeNMEA(char *buff)
{
  uint8_t p = 1;
  uint8_t crc = buff[p++];
  while ( buff[p] != '*' )
    crc ^= buff[p++];

  sprintf(buff+p+1, "%.2X\r\n", crc);
}

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



#ifndef NMEAENCODER_HPP_
#define NMEAENCODER_HPP_

#include <vector>
#include <string>
#include "RXPacket.hpp"
using namespace std;


class NMEAEncoder
{
public:
  NMEAEncoder();
  virtual ~NMEAEncoder();

  void encode(RXPacket &packet, vector<string> &sentences);
private:
  uint8_t nmeaCRC(const char* buff);
private:
  uint8_t mSequence;
};

#endif /* NMEAENCODER_HPP_ */

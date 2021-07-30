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

#ifndef TXPACKET_HPP_
#define TXPACKET_HPP_

#include "ObjectPool.hpp"
#include "AISChannels.h"
#include "config.h"
#include <time.h>

class TXPacket
{
public:
  TXPacket ();
  ~TXPacket();

  void addBit(uint8_t bit);
  void pad();
  uint16_t size();

  // Iterator pattern for transmitting bit-by-bit
  bool eof();
  uint8_t nextBit();
  VHFChannel channel();

  void setTimestamp(time_t t);
  time_t timestamp();

  void setMessageType(const char*);
  const char *messageType();

  void configure(VHFChannel channel);
  void reset();

  void configureForTesting(VHFChannel channel, uint16_t numBits);
  bool canRampDown();
  bool isTestPacket();
private:
  uint8_t mPacket[MAX_AIS_TX_PACKET_SIZE/8+1];
  uint16_t mSize;
  uint16_t mPosition;
  VHFChannel mChannel;
  time_t mTimestamp;
  char mMessageType[4];
  bool mTestPacket = false;
};


class TXPacketPool
{
public:
  static TXPacketPool &instance();
  void init();

  TXPacket *newTXPacket(VHFChannel channel);
  void deleteTXPacket(TXPacket*);
private:
  ObjectPool<TXPacket> *mPool;
};



#endif /* TXPACKET_HPP_ */

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


#include <cassert>
#include <cstring>
#include "RXPacket.hpp"


RXPacket::RXPacket ()
{
  reset();
}

RXPacket::~RXPacket ()
{
}

void RXPacket::setChannel(VHFChannel channel)
{
  mChannel = channel;
}

VHFChannel RXPacket::channel() const
{
  return mChannel;
}

RXPacket::RXPacket(const RXPacket &copy)
{
  memcpy(mPacket, copy.mPacket, sizeof mPacket);
  mSize = copy.mSize;
  mCRC = copy.mCRC;
  mRSSI = copy.mRSSI;
  mType = copy.mType;
  mRI = copy.mRI;
  mMMSI = copy.mMMSI;
  mSlot = copy.mSlot;
  mChannel = copy.mChannel;
}

RXPacket &RXPacket::operator =(const RXPacket &copy)
{
  memcpy(mPacket, copy.mPacket, sizeof mPacket);
  mSize = copy.mSize;
  mCRC = copy.mCRC;
  mType = copy.mType;
  mRSSI = copy.mRSSI;
  mRI = copy.mRI;
  mMMSI = copy.mMMSI;
  mChannel = copy.mChannel;
  mSlot = copy.mSlot;
  return *this;
}

void RXPacket::reset()
{
  mType = 0;
  mRI = 0;
  mMMSI = 0;
  mSize = 0;
  mCRC = 0xffff;
  mSlot = 0xffffffff;
  mRSSI = 0;
  memset(mPacket, 0, sizeof mPacket);
}

void RXPacket::setSlot(uint32_t slot)
{
  mSlot = slot;
}

uint32_t RXPacket::slot() const
{
  return mSlot;
}


void RXPacket::setRSSI(uint8_t rssi)
{
  mRSSI = rssi;
}

uint8_t RXPacket::rssi() const
{
  return mRSSI;
}


void RXPacket::addBit(uint8_t bit)
{
  ASSERT(mSize < MAX_AIS_RX_PACKET_SIZE);

  uint16_t index = mSize / 8;
  uint8_t offset = mSize % 8;

  if ( bit )
    mPacket[index] |= ( 1 << offset );
  else
    mPacket[index] &= ~( 1 << offset );

  ++mSize;
}

uint8_t RXPacket::bit(uint16_t pos) const
{
  if ( pos < mSize ) {
      uint16_t index = pos / 8;
      uint8_t offset = pos % 8;

      return (mPacket[index] & (1 << offset)) != 0;
  }
  else
    return 0;
}

uint32_t RXPacket::bits(uint16_t pos, uint8_t count) const
{
  ASSERT(count <= 32);
  uint32_t result = 0;

  for ( uint16_t i = pos; i < pos+count; ++i ) {
      result <<= 1;
      result |= bit(i);
  }

  return result;
}

void RXPacket::addBitCRC(uint8_t data)
{
  if ( (data ^ mCRC) & 0x0001 )
    mCRC = (mCRC >> 1) ^ 0x8408;
  else
    mCRC >>= 1;
}

void RXPacket::addByte(uint8_t byte)
{
  // The payload is LSB (inverted MSB bytes). This brings it back into MSB format

      addBit(byte & 0x01);
  addBit(byte & 0x02);
  addBit(byte & 0x04);
  addBit(byte & 0x08);
  addBit(byte & 0x10);
  addBit(byte & 0x20);
  addBit(byte & 0x40);
  addBit(byte & 0x80);

  // Now we can update our CRC in MSB order which is how it was calculated during encoding by the sender ...
  addBitCRC((byte & 0x80) >> 7);
  addBitCRC((byte & 0x40) >> 6);
  addBitCRC((byte & 0x20) >> 5);
  addBitCRC((byte & 0x10) >> 4);
  addBitCRC((byte & 0x08) >> 3);
  addBitCRC((byte & 0x04) >> 2);
  addBitCRC((byte & 0x02) >> 1);
  addBitCRC(byte & 0x01);

}


uint16_t RXPacket::size() const
{
  return mSize;
}


bool RXPacket::isBad() const
{
  /*
   * We don't anticipate anything less than 168 + 16 = 184 bits
   */

  //return mSize < 184;
  return mSize < 32;
}

uint16_t RXPacket::crc() const
{
  return mCRC;
}

void RXPacket::discardCRC()
{
  if ( mCRC == 0xffff )
    return;
  mSize -= 16;
  mCRC = 0xffff;

  // Explicitly set those bits to zero, no matter how they align
  for ( uint8_t i = 0; i < 16; ++i )
    addBit(0);

  mSize -= 16;
}

void RXPacket::addFillBits(uint8_t numBits)
{
  for ( uint8_t i = 0; i < numBits; ++i )
    addBit(0);
}



bool RXPacket::checkCRC() const
{
  //uint16_t rcrc = ((mCRC & 0xff00) >> 8) | ((mCRC & 0x00ff) << 8);
  //trace_printf("%.4x %.4x %.4x\n", mCRC, ~(mCRC), ~(rcrc));
  return mCRC == 0xf0b8;

}

uint8_t RXPacket::messageType() const
{
  if ( mType )
    return mType;

  for ( int i = 0; i < 6; ++i ) {
      mType <<= 1;
      mType |= bit(i);
  }

  return mType;
}

uint8_t RXPacket::repeatIndicator() const
{
  if ( mRI )
    return mRI;

  mRI = bit(6) << 1 | bit(7);
  return mRI;
}

uint32_t RXPacket::mmsi() const
{
  if ( mMMSI )
    return mMMSI;

  for ( int i = 8; i < 38; ++i ) {
      mMMSI <<= 1;
      mMMSI |= bit(i);
  }

  return mMMSI;
}





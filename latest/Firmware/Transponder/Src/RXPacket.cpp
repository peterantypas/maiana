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

//#define memcpy my_on_steroids_memcpy


RXPacket::RXPacket ()
{
  reset();
}

RXPacket::~RXPacket ()
{
}

void RXPacket::setChannel(VHFChannel channel)
{
  mState.mChannel = channel;
}

VHFChannel RXPacket::channel() const
{
  return mState.mChannel;
}

#if 1
RXPacket::RXPacket(const RXPacket &copy)
{
#if 0
  memcpy(mPacket, copy.mPacket, sizeof mPacket);
  mSize = copy.mSize;
  mCRC = copy.mCRC;
  mRSSI = copy.mRSSI;
  mType = copy.mType;
  mRI = copy.mRI;
  mMMSI = copy.mMMSI;
  mSlot = copy.mSlot;
  mChannel = copy.mChannel;
#endif
  memcpy(&mState, &copy.mState, sizeof mState);
}

RXPacket &RXPacket::operator =(const RXPacket &copy)
{
#if 0
  memcpy(mPacket, copy.mPacket, sizeof mPacket);
  mSize = copy.mSize;
  mCRC = copy.mCRC;
  mType = copy.mType;
  mRSSI = copy.mRSSI;
  mRI = copy.mRI;
  mMMSI = copy.mMMSI;
  mChannel = copy.mChannel;
  mSlot = copy.mSlot;
#endif
  memcpy(&mState, &copy.mState, sizeof mState);
  return *this;
}
#endif

void RXPacket::reset()
{
  mState = {{0}, 0, 0xffff, 0, 0, 0, 0xffffffff, CH_18, 0};
#if 0
  mType = 0;
  mRI = 0;
  mMMSI = 0;
  mSize = 0;
  mCRC = 0xffff;
  mSlot = 0xffffffff;
  mRSSI = 0;
  memset(mPacket, 0, sizeof mPacket);
#endif
}

void RXPacket::setSlot(uint32_t slot)
{
  mState.mSlot = slot;
}

uint32_t RXPacket::slot() const
{
  return mState.mSlot;
}


void RXPacket::setRSSI(uint8_t rssi)
{
  mState.mRSSI = rssi;
}

uint8_t RXPacket::rssi() const
{
  return mState.mRSSI;
}


void RXPacket::addBit(uint8_t bit)
{
  //ASSERT(mSize < MAX_AIS_RX_PACKET_SIZE);

  uint16_t index = mState.mSize / 8;
  uint8_t offset = mState.mSize % 8;

  if ( bit )
    mState.mPacket[index] |= ( 1 << offset );
  else
    mState.mPacket[index] &= ~( 1 << offset );

  ++mState.mSize;
}

uint8_t RXPacket::bit(uint16_t pos) const
{
  if ( pos < mState.mSize ) {
      uint16_t index = pos / 8;
      uint8_t offset = pos % 8;

      return (mState.mPacket[index] & (1 << offset)) != 0;
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
  if ( (data ^ mState.mCRC) & 0x0001 )
    mState.mCRC = (mState.mCRC >> 1) ^ 0x8408;
  else
    mState.mCRC >>= 1;
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
  return mState.mSize;
}


bool RXPacket::isBad() const
{
  /*
   * We don't anticipate anything less than 168 + 16 = 184 bits
   */

  //return mSize < 184;
  return mState.mSize < 32;
}

uint16_t RXPacket::crc() const
{
  return mState.mCRC;
}

void RXPacket::discardCRC()
{
  if ( mState.mCRC == 0xffff )
    return;
  mState.mSize -= 16;
  mState.mCRC = 0xffff;

  // Explicitly set those bits to zero, no matter how they align
  for ( uint8_t i = 0; i < 16; ++i )
    addBit(0);

  mState.mSize -= 16;
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
  return mState.mCRC == 0xf0b8;

}

uint8_t RXPacket::messageType() const
{
  if ( mState.mType )
    return mState.mType;

  for ( int i = 0; i < 6; ++i ) {
      mState.mType <<= 1;
      mState.mType |= bit(i);
  }

  return mState.mType;
}

uint8_t RXPacket::repeatIndicator() const
{
  if ( mState.mRI )
    return mState.mRI;

  mState.mRI = bit(6) << 1 | bit(7);
  return mState.mRI;
}

uint32_t RXPacket::mmsi() const
{
  if ( mState.mMMSI )
    return mState.mMMSI;

  for ( int i = 8; i < 38; ++i ) {
      mState.mMMSI <<= 1;
      mState.mMMSI |= bit(i);
  }

  return mState.mMMSI;
}





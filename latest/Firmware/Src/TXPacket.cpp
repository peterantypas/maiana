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


#include <cstring>
#include <cassert>
#include "TXPacket.hpp"
#include <stdlib.h>


TXPacket::TXPacket ()
{
  reset();
}


void TXPacket::configure(VHFChannel channel)
{
  mTestPacket = false;
  mChannel  = channel;
  memset(mMessageType, 0, sizeof mMessageType);
}

void TXPacket::configureForTesting(VHFChannel channel, uint16_t numBits)
{
  mTestPacket = true;
  mChannel  = channel;
  strcpy(mMessageType, "00");
  mSize = numBits;
}

TXPacket::~TXPacket ()
{
}

void TXPacket::reset()
{
  mSize      = 0;
  mPosition  = 0;
  mChannel   = CH_87;
  mTimestamp = 0;
  memset(mPacket, 0, sizeof mPacket);
}

uint16_t TXPacket::size()
{
  return mSize;
}

void TXPacket::setTimestamp(time_t t)
{
  mTimestamp = t;
}

time_t TXPacket::timestamp()
{
  return mTimestamp;
}

void TXPacket::setMessageType(const char *t)
{
  strlcpy(mMessageType, t, sizeof mMessageType);
}

bool TXPacket::isTestPacket()
{
  return mTestPacket;
}

const char *TXPacket::messageType()
{
  return mMessageType;
}

VHFChannel TXPacket::channel()
{
  return mChannel;
}

void TXPacket::addBit(uint8_t bit)
{
  ASSERT(mSize < MAX_AIS_TX_PACKET_SIZE);

  uint16_t index = mSize / 8;
  uint8_t offset = mSize % 8;

  if ( bit )
    mPacket[index] |= ( 1 << offset );

  ++mSize;
}

void TXPacket::pad()
{
  uint16_t rem = 8 - mSize % 8;
  for ( uint16_t i = 0; i < rem; ++i )
    addBit(0);
}

bool TXPacket::eof()
{
  if ( mSize == 0 )
    return true;

  return mPosition > mSize - 1;
}

bool TXPacket::canRampDown()
{
  return mPosition == mSize - 3;
}

uint8_t TXPacket::nextBit()
{
  ASSERT(mPosition < mSize);
  if ( mTestPacket )
    {
      ++mPosition;
      return rand() % 2;
    }
  else
    {
      uint16_t index = mPosition / 8;
      uint8_t offset = mPosition % 8;
      ++mPosition;
      return (mPacket[index] & ( 1 << offset )) != 0;
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// TXPacketPool
//
///////////////////////////////////////////////////////////////////////////////

TXPacketPool &TXPacketPool::instance()
{
  static TXPacketPool __instance;
  return __instance;
}

void TXPacketPool::init()
{
  mPool = new ObjectPool<TXPacket>(4);
}

TXPacket *TXPacketPool::newTXPacket(VHFChannel channel)
{
  TXPacket *p = mPool->get();
  if ( !p )
    return p;

  p->reset();
  p->configure(channel);
  return p;
}

void TXPacketPool::deleteTXPacket(TXPacket* p)
{
  ASSERT(p);
  mPool->put(p);
}




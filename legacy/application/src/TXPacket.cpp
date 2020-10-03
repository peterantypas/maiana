/*
 * TXPacket.cpp
 *
 *  Created on: Dec 12, 2015
 *      Author: peter
 */

#include <cstring>
#include <cassert>
#include "TXPacket.hpp"

TXPacket::TXPacket ()
{
    reset();
}

void TXPacket::configure(VHFChannel channel, time_t txTime)
{
    mChannel  = channel;
    mTXTime   = txTime;
}

TXPacket::~TXPacket ()
{
}

void TXPacket::reset()
{
    mSize     = 0;
    mPosition = 0;
    mChannel  = CH_87;
    mTXTime   = 0;
    memset(mPacket, 0, sizeof mPacket);
}

uint16_t TXPacket::size()
{
    return mSize;
}

time_t TXPacket::txTime()
{
    return mTXTime;
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

uint8_t TXPacket::nextBit()
{
    ASSERT(mPosition < mSize);

    uint16_t index = mPosition / 8;
    uint8_t offset = mPosition % 8;

    ++mPosition;
    return (mPacket[index] & ( 1 << offset )) != 0;
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

TXPacket *TXPacketPool::newTXPacket(VHFChannel channel, time_t txTime)
{
    TXPacket *p = mPool->get();
    if ( !p )
        return p;

    p->reset();
    p->configure(channel, txTime);
    return p;
}

void TXPacketPool::deleteTXPacket(TXPacket* p)
{
    ASSERT(p);
    mPool->put(p);
}




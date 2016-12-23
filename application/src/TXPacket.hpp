/*
 * TXPacket.hpp
 *
 *  Created on: Dec 12, 2015
 *      Author: peter
 */

#ifndef TXPACKET_HPP_
#define TXPACKET_HPP_

#include <inttypes.h>
#include <time.h>

#include "globals.h"
#include "ObjectPool.hpp"
#include "AISChannels.h"


class TXPacket
{
public:
    friend class TXPacketPool;

    TXPacket ();

    void addBit(uint8_t bit);
    void pad();
    uint16_t size();

    // Iterator pattern for transmitting bit-by-bit
    bool eof();
    uint8_t nextBit();
    VHFChannel channel();

    void setTimestamp(time_t t);
    time_t timestamp();
private:
    ~TXPacket ();
    void configure(VHFChannel channel);
    void reset();
private:
    uint8_t mPacket[MAX_AIS_TX_PACKET_SIZE/8+1];
    uint16_t mSize;
    uint16_t mPosition;
    VHFChannel mChannel;
    time_t mTimestamp;
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

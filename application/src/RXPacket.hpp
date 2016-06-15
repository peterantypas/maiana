/*
 * AISPacket.hpp
 *
 *  Created on: Dec 6, 2015
 *      Author: peter
 */

#ifndef RXPACKET_HPP_
#define RXPACKET_HPP_

#include <inttypes.h>
#include "ObjectPool.hpp"
#include "globals.h"
#include "AISChannels.h"

using namespace std;

class RXPacket
{
public:
    RXPacket ();
    ~RXPacket ();
    RXPacket(const RXPacket &copy);
    RXPacket &operator=(const RXPacket &copy);
public:

    void setSlot(uint32_t slot);

    void setChannel(VHFChannel channel);
    VHFChannel channel() const;


    void discardCRC();
    void addFillBits(uint8_t numBits);

    void addByte(uint8_t byte);

    uint16_t size() const;
    uint8_t bit(uint16_t pos) const;
    uint32_t bits(uint16_t pos, uint8_t count) const;

    uint16_t crc() const;
    bool checkCRC() const;
    bool isBad() const;
    void reset();



    // Every AIS message contains these 3 attributes at a minimum, so we expose them at the packet level
    uint8_t messageType() const;
    uint8_t repeatIndicator() const;
    uint32_t mmsi() const;

    // These are link-level attributes
    uint32_t slot() const;
    uint8_t rssi() const;
    void setRSSI(uint8_t);
private:
    void addBit(uint8_t bit);
    void addBitCRC(uint8_t bit);
private:
    uint8_t mPacket[MAX_AIS_RX_PACKET_SIZE/8+1];
    uint16_t mSize;
    uint16_t mCRC;
    mutable uint8_t mType;
    mutable uint8_t mRI;
    mutable uint32_t mMMSI;
    uint32_t mSlot;
    VHFChannel mChannel;
    uint8_t mRSSI;
};

#if 0
class RXPacketPool
{
public:
    static RXPacketPool &instance();

    void init();

    RXPacket *newRXPacket();
    void deleteRXPacket(RXPacket*);

private:
    ObjectPool<RXPacket> *mPool;

};
#endif

#endif /* RXPACKET_HPP_ */

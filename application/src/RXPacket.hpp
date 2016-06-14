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
    friend class RXPacketPool;
public:
    RXPacket ();
private:
    RXPacket(const RXPacket &copy);
    RXPacket &operator=(const RXPacket &copy);
    ~RXPacket ();
public:

    //void setStuffed(bool);
    void setSlot(uint32_t slot);

    void setChannel(VHFChannel channel);
    VHFChannel channel();


    void discardCRC();
    void addFillBits(uint8_t numBits);

    void addByte(uint8_t byte);

    uint16_t size();
    uint8_t bit(uint16_t pos);
    uint32_t bits(uint16_t pos, uint8_t count);

    uint16_t crc();
    bool checkCRC();
    bool isBad();
    void reset();



    // Every AIS message contains these 3 attributes at a minimum, so we expose them at the packet level
    uint8_t messageType();
    uint8_t repeatIndicator();
    uint32_t mmsi();

    // These are link-level attributes
    uint32_t slot();
    uint8_t rssi();
    void setRSSI(uint8_t);
private:
    void addBit(uint8_t bit);
    void addBitCRC(uint8_t bit);
private:
    uint8_t mPacket[MAX_AIS_RX_PACKET_SIZE/8+1];
    uint16_t mSize;
    uint16_t mCRC;
    uint8_t mType;
    uint8_t mRI;
    uint32_t mMMSI;
    uint32_t mSlot;
    VHFChannel mChannel;
    uint8_t mRSSI;
};


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


#endif /* RXPACKET_HPP_ */

/*
 * PacketProcessor.hpp
 *
 *  Created on: Dec 7, 2015
 *      Author: peter
 */

#ifndef RXPACKETPROCESSOR_HPP_
#define RXPACKETPROCESSOR_HPP_

#include "Events.hpp"
#include "NMEAEncoder.hpp"
#include <set>


class RXPacketProcessor : public EventConsumer
{
public:
    RXPacketProcessor ();
    virtual
    ~RXPacketProcessor ();

    void processEvent(Event *e);
private:
    NMEAEncoder mEncoder;
    time_t mLastDumpTime;
    uint32_t mGoodCount;
    uint32_t mBadCRCCount;
    uint32_t mInvalidCount;
    double mLat;
    double mLng;
    std::set<uint32_t> mUniqueMMSIs;
};

#endif /* RXPACKETPROCESSOR_HPP_ */

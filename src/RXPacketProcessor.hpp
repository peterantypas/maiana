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
#include "EEPROM.hpp"
#include <vector>

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
    std::vector<std::string> mSentences;
    StationData mStationData;
};

#endif /* RXPACKETPROCESSOR_HPP_ */

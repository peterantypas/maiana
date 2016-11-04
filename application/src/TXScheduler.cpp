/*
 * TXScheduler.cpp
 *
 *  Created on: Dec 12, 2015
 *      Author: peter
 */

#include "TXScheduler.hpp"
#include "EventQueue.hpp"
#include "AISMessages.hpp"
#include "TXPacket.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>
#include "RadioManager.hpp"
#include "ChannelManager.hpp"
#include "printf2.h"

using namespace std;

TXScheduler::TXScheduler ()
{
    EventQueue::instance().addObserver(this, GPS_FIX_EVENT | CLOCK_EVENT);
    mPositionReportChannel = CH_87;
    mStaticDataChannel = CH_87;
    mUTC = 0;
    mAvgSpeed = 0.0;
    mTesting = false;
    mLast18Time = 0;
    mLast24Time = 0;
}


TXScheduler::~TXScheduler ()
{
}

void TXScheduler::startTXTesting()
{
    mTesting = true;
}

void TXScheduler::processEvent(const Event &e)
{
#ifndef ENABLE_TX
    return;
#endif

    switch(e.type) {
        case GPS_FIX_EVENT: {
            if ( mTesting )
                return;

            // We do not schedule transmissions if the ChannelManager is not sure what channels are in use yet
            if ( !ChannelManager::instance().channelsDetermined() )
                return;

            // A class B transponder only transmits when its internal GPS is working, so we tie these to GPS updates.
            if ( !RadioManager::instance().initialized() || mUTC == 0 )
              return;

            StationData stationData;
            EEPROM::instance().readStationData(stationData);
            if ( stationData.flags & STATION_RX_ONLY )
                return;

            // Using a moving average of SOG to determine transmission rate
            double alpha = 0.2;
            mAvgSpeed = mAvgSpeed * (1.0 - alpha) + e.gpsFix.speed * alpha;

            if ( mUTC - mLast18Time > positionReportTimeInterval() ) {
                TXPacket *p1 = TXPacketPool::instance().newTXPacket(mPositionReportChannel, mUTC);
                if ( !p1 ) {
                    printf2("Unable to allocate TX packet for message 18, will try again later\r\n");
                    break;
                }

                AISMessage18 msg;
                msg.latitude    = e.gpsFix.lat;
                msg.longitude   = e.gpsFix.lng;
                msg.sog         = e.gpsFix.speed;
                msg.cog         = e.gpsFix.cog;
                msg.utc         = e.gpsFix.utc;
                msg.encode (stationData, *p1);
                RadioManager::instance ().scheduleTransmission (p1);

                // Our next position report should be on the other channel
                mPositionReportChannel = RadioManager::instance().alternateChannel(mPositionReportChannel);
                mLast18Time = mUTC;
            }


            if ( mUTC - mLast24Time > MSG_24_TX_INTERVAL ) {
                TXPacket *p2 = TXPacketPool::instance().newTXPacket(mStaticDataChannel, mUTC+2);
                if ( !p2 ) {
                    printf2("Unable to allocate TX packet for 24A\r\n");
                    break;
                }

                AISMessage24A msg2;
                msg2.encode(stationData, *p2);
                RadioManager::instance().scheduleTransmission(p2);

                TXPacket *p3 = TXPacketPool::instance().newTXPacket(mStaticDataChannel, mUTC+7);
                if ( !p3 ) {
                    printf2("Unable to allocate TX packet for 24B\r\n");
                    break;
                }

                AISMessage24B msg3;
                msg3.encode(stationData, *p3);
                RadioManager::instance().scheduleTransmission(p3);

                // Our next static data report should be on the other channel
                mStaticDataChannel = RadioManager::instance().alternateChannel(mStaticDataChannel);
                mLast24Time = mUTC;
            }

            break;
        }
        case CLOCK_EVENT: {
            // This is reliable and independent of GPS update frequency which could change to something other than 1Hz
            mUTC = e.clock.utc;
            if ( RadioManager::instance().initialized() && mTesting && mUTC % 1 == 0 ) {
                scheduleTestPacket();
                printf2("Scheduled test packet\r\n");
            }
            break;
        }
        default:
            break;
    }

}

time_t TXScheduler::positionReportTimeInterval()
{
    // As a class B "CS" transponder, we transmit every 3 minutes if speed is < 2 knots, otherwise 30 seconds.
    if ( mAvgSpeed < 2.0 )
        return 180;

    return DEFAULT_TX_INTERVAL;
}

void TXScheduler::scheduleTestPacket()
{
    VHFChannel channel = CH_84;
    if ( rand() % 2 == 0 )
        channel = CH_85;
    TXPacket *p = TXPacketPool::instance().newTXPacket(channel, mUTC);
    if ( !p )
        return;

    for ( int i = 0; i < MAX_AIS_TX_PACKET_SIZE; ++i ) {
        p->addBit(rand() % 2);
    }
    RadioManager::instance().scheduleTransmission(p);
}



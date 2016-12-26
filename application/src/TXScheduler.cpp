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

TXScheduler &TXScheduler::instance()
{
    static TXScheduler __instance;
    return __instance;
}

TXScheduler::TXScheduler ()
{
    EventQueue::instance().addObserver(this, GPS_FIX_EVENT | CLOCK_EVENT | INTERROGATION_EVENT);
    mPositionReportChannel = CH_87;
    mStaticDataChannel = CH_87;
    mUTC = 0;
    mAvgSpeed = 0.0;
    mTesting = false;
    mLast18Time = 0;
    mLast24Time = 0;
    if ( Configuration::instance().readStationData(mStationData) ) {
        printf2("Successfully loaded Station Data from Flash\r\n");
    }
    else {
        printf2("Failed to read Station Data from Flash!!!\r\n");
    }
}

void TXScheduler::init()
{
}

TXScheduler::~TXScheduler ()
{
}

void TXScheduler::startTXTesting()
{
    printf2("TX Scheduler starting testing\r\n");
    mTesting = true;
}

void TXScheduler::processEvent(const Event &e)
{
#ifndef ENABLE_TX
    return;
#endif

    switch(e.type) {
    case GPS_FIX_EVENT: {
        mLastGPSFix = e.gpsFix;
        
        if ( mTesting )
            return;

        // We do not schedule transmissions if the ChannelManager is not sure what channels are in use yet
        if ( !ChannelManager::instance().channelsDetermined() )
            return;

        // A class B transponder only transmits when its internal GPS is working, so we tie these to GPS updates.
        if ( !RadioManager::instance().initialized() || mUTC == 0 )
            return;

#ifdef TX_TEST_MODE
        return;
#endif
            
        // Using a moving average of SOG to determine transmission rate
        static double alpha = 0.2;
        mAvgSpeed = mAvgSpeed * (1.0 - alpha) + mLastGPSFix.speed * alpha;

        if ( mUTC - mLast18Time > positionReportTimeInterval() ) {
            queueMessage18(mPositionReportChannel);
            // Our next position report should be on the other channel
            mPositionReportChannel = RadioManager::instance().alternateChannel(mPositionReportChannel);
            mLast18Time = mUTC;
        }
        
        if ( mUTC - mLast24Time > MSG_24_TX_INTERVAL ) {
            queueMessage24(mStaticDataChannel);
            // Our next static data report should be on the other channel
            mStaticDataChannel = RadioManager::instance().alternateChannel(mStaticDataChannel);
            mLast24Time = mUTC;
        }

        break;
    }
    case CLOCK_EVENT: {
        // This is reliable and independent of GPS update frequency which could change to something other than 1Hz
        mUTC = e.clock.utc;
#ifdef TX_TEST_MODE
        if ( RadioManager::instance().initialized() && mTesting && mUTC % 10 == 0 ) {
            scheduleTestPacket();
            printf2("Scheduled test packet\r\n");
        }
#endif
        break;
    }
    case INTERROGATION_EVENT:
        if ( e.interrogation.messageType == 18 )
            queueMessage18(e.interrogation.channel);

        if ( e.interrogation.messageType == 24 )
            queueMessage24(e.interrogation.channel);
        break;
    default:
        break;
    }

}

void TXScheduler::queueMessage18(VHFChannel channel)
{
    // If we don't have valid station data we don't do anything
    if ( mStationData.magic != STATION_DATA_MAGIC )
        return;

        TXPacket *p1 = TXPacketPool::instance().newTXPacket(channel);
    if ( !p1 ) {
        printf2("Unable to allocate TX packet for message 18, will try again later\r\n");
        return;
    }
    
    AISMessage18 msg;
    msg.latitude    = mLastGPSFix.lat;
    msg.longitude   = mLastGPSFix.lng;
    msg.sog         = mLastGPSFix.speed;
    msg.cog         = mLastGPSFix.cog;
    msg.utc         = mLastGPSFix.utc;
    msg.encode (mStationData, *p1);
    
    RadioManager::instance ().scheduleTransmission (p1);
}

void TXScheduler::queueMessage24(VHFChannel channel)
{
    // If we don't have valid station data we don't do anything
    if ( mStationData.magic != STATION_DATA_MAGIC )
        return;
    
    TXPacket *p2 = TXPacketPool::instance().newTXPacket(channel);
    if ( !p2 ) {
        printf2("Unable to allocate TX packet for 24A\r\n");
        return;
    }
    
    AISMessage24A msg2;
    msg2.encode(mStationData, *p2);
    
    RadioManager::instance().scheduleTransmission(p2);
    
    TXPacket *p3 = TXPacketPool::instance().newTXPacket(channel);
    if ( !p3 ) {
        printf2("Unable to allocate TX packet for 24B\r\n");
        return;
    }
    
    AISMessage24B msg3;
    msg3.encode(mStationData, *p3);
    RadioManager::instance().scheduleTransmission(p3);
    
}

time_t TXScheduler::positionReportTimeInterval()
{
    // As a class B "CS" transponder, we transmit at a rate based on our speed (2 knots is the threshold)
    if ( mAvgSpeed < 2.0 )
        return MAX_MSG_18_TX_INTERVAL;

    return MIN_MSG_18_TX_INTERVAL;
}

#ifdef TX_TEST_MODE
void TXScheduler::scheduleTestPacket()
{
    VHFChannel channel = CH_84;
    if ( rand() % 2 == 0 )
        channel = CH_85;

    TXPacket *p = TXPacketPool::instance().newTXPacket(channel);
    if ( !p ) {
        printf2("Ooops! Out of TX packets :(\r\n");
        return;
    }

    for ( int i = 0; i < 240; ++i ) {
        p->addBit(rand() % 2);
    }
    RadioManager::instance().scheduleTransmission(p);
}
#endif



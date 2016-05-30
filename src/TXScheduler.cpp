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
    mTesting = false;

    EEPROM::instance().readStationData(mStationData);
    printf2("Read station data from EEPROM:\r\n");
    printf2("MMSI: %d\r\n", mStationData.mmsi);
    printf2("CS: %s\r\n", mStationData.callsign);
    printf2("Name: %s\r\n", mStationData.name);
}


TXScheduler::~TXScheduler ()
{
}

void TXScheduler::startTXTesting()
{
    mTesting = true;
}

void TXScheduler::processEvent(Event *event)
{
    //printf2("-> TXScheduler::processEvent()\r\n");
#ifndef ENABLE_TX
    return;
#endif

    switch(event->type()) {
        case GPS_FIX_EVENT: {
            if ( mTesting )
                return;

            //if ( Radio::instance().isCalibratingRSSI() )
              //  return;

            GPSFIXEvent *gfe = static_cast<GPSFIXEvent*> (event);

            //printf2("UTC: %d\r\n", mUTC);

            // We do not schedule transmissions if the ChannelManager is not sure what channels are in use yet
            if ( !ChannelManager::instance().channelsDetermined() )
                return;

            // TODO: This is UGLY. Stop using modulus and start counting from previous timestamps

            // A class B only transmits when its internal GPS is working, so we tie these to GPS updates.
            if ( RadioManager::instance().initialized() && mUTC && (mUTC % DEFAULT_TX_INTERVAL) == 0) {
                TXPacket *p1 = TXPacketPool::instance().newTXPacket(mPositionReportChannel, mUTC);
                if ( !p1 ) {
                    printf2("Unable to allocate TX packet for message 18, will try again later\r\n");
                    break;
                }
                AISMessage18 msg;

                msg.latitude    = gfe->mLat;
                msg.longitude   = gfe->mLng;
                msg.sog         = gfe->mSpeed;
                msg.cog         = gfe->mCOG;
                msg.utc         = gfe->mUTC;

                msg.encode (mStationData, *p1);
                RadioManager::instance ().scheduleTransmission (p1);

                // Our next position report should be on the other channel
                mPositionReportChannel = RadioManager::instance().alternateChannel(mPositionReportChannel);
            }


            if ( RadioManager::instance().initialized() && mUTC && (mUTC % MSG_24_TX_INTERVAL) == 0 ) {
                TXPacket *p2 = TXPacketPool::instance().newTXPacket(mStaticDataChannel, mUTC+2);
                if ( !p2 ) {
                    printf2("Unable to allocate TX packet for 24A\r\n");
                    break;
                }
                AISMessage24A msg2;
                msg2.encode(mStationData, *p2);
                RadioManager::instance().scheduleTransmission(p2);

                TXPacket *p3 = TXPacketPool::instance().newTXPacket(mStaticDataChannel, mUTC+7);
                if ( !p3 ) {
                    printf2("Unable to allocate TX packet for 24B\r\n");
                    break;
                }

                AISMessage24B msg3;
                msg3.encode(mStationData, *p3);
                RadioManager::instance().scheduleTransmission(p3);

                // Our next static data report should be on the other channel
                mStaticDataChannel = RadioManager::instance().alternateChannel(mStaticDataChannel);
            }

            break;
        }
        case CLOCK_EVENT: {
            // This is reliable and independent of GPS update frequency which could change to something other than 1Hz
            ClockEvent *c = static_cast<ClockEvent*>(event);
            mUTC = c->mTime;
            if ( RadioManager::instance().initialized() && mTesting && mUTC % 1 == 0 ) {
                scheduleTestPacket();
                printf2("Scheduled test packet\r\n");
            }
            break;
        }
        default:
            break;
    }

    //printf2("<- TXScheduler::processEvent()\r\n");
}

void TXScheduler::scheduleTestPacket()
{
/*
    TXPacket *p = TXPacketPool::instance().newTXPacket(AIS_CHANNELS[rand() % 2 + 18].vhf, mUTC);
    if ( !p )
        return;

    for ( int i = 0; i < MAX_AIS_TX_PACKET_SIZE; ++i ) {
        p->addBit(rand() % 2);
    }
    RadioManager::instance().scheduleTransmission(p);
*/
}



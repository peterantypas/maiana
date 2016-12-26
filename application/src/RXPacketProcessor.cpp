/*
 * PacketProcessor.cpp
 *
 *  Created on: Dec 7, 2015
 *      Author: peter
 */

#include <stdio.h>
#include "RXPacketProcessor.hpp"
#include "AISMessages.hpp"
#include "DataTerminal.hpp"
#include "EventQueue.hpp"
#include "Utils.hpp"
#include "LEDManager.hpp"
#include "MetricUnits.hpp"
#include "printf2.h"
#include "AISChannels.h"

static char __buff[128];

RXPacketProcessor::RXPacketProcessor ()
    : mLastDumpTime(0), mGoodCount(0), mBadCRCCount(0), mInvalidCount(0), mLat(-200), mLng(-200)
{
    mSentences.reserve(4); // We're not going to need more than 2 sentences for the longest AIS message we report ...
    Configuration::instance().readStationData(mStationData);
    EventQueue::instance().addObserver(this, AIS_PACKET_EVENT | CLOCK_EVENT | GPS_FIX_EVENT);
}

RXPacketProcessor::~RXPacketProcessor ()
{
    // Should never be called
}

void RXPacketProcessor::processEvent(const Event &e)
{
    //printf2("-> RXPacketProcessor::processEvent()\r\n");
    switch(e.type) {
        case GPS_FIX_EVENT: {
            //GPSFIXEvent *gfe = static_cast<GPSFIXEvent*> (e);
            mLat = e.gpsFix.lat;
            mLng = e.gpsFix.lng;
            break;
        }
        case CLOCK_EVENT: {
            //ClockEvent *pe = static_cast<ClockEvent*>(e);
            if ( mLastDumpTime == 0 ) {
                mLastDumpTime = e.clock.utc;
            }
            else if ( e.clock.utc - mLastDumpTime >= 60 ) {
                mLastDumpTime = e.clock.utc;
                float yield = (float)mGoodCount / (float)(mGoodCount+mBadCRCCount+mInvalidCount);
                printf2("\r\n");
                printf2("[Yield: %.1fpct, Valid: %d, Wrong CRC: %d, Malformed: %d]\r\n", yield*100.0,
                        mGoodCount, mBadCRCCount, mInvalidCount);
                printf2("[Unique MMSIs: %d]\r\n", mUniqueMMSIs.size());
                printf2("\r\n");
                mUniqueMMSIs.clear();
                mBadCRCCount = 0;
                mInvalidCount = 0;
                mGoodCount = 0;
            }


            break;
        }
        case AIS_PACKET_EVENT: {
            //AISPacketEvent *pe = static_cast<AISPacketEvent*>(e);
            if ( e.rxPacket.isBad() ) {
                ++mInvalidCount;
                break;
            }


            if (e.rxPacket.checkCRC ()) {
                ++mGoodCount;

                mUniqueMMSIs.insert (e.rxPacket.mmsi ());
                switch (e.rxPacket.messageType()) {
                    case 1:
                    case 2:
                    case 3: {
                        AISMessage123 msg;
                        if (msg.decode (e.rxPacket)) {
                            double distance = Utils::haversineDistance (mLat, mLng, msg.latitude, msg.longitude);
                            double miles = distance / METERS_PER_NAUTICAL_MILE;

                            printf2 (
                                    "RSSI: %.2x, Ch: %c, Type: %d, MMSI: %d, Speed: %.1f kts, Pos: %.5f,%.5f, Dist: %.1f NM\r\n",
                                    e.rxPacket.rssi(),
                                    AIS_CHANNELS[e.rxPacket.channel()].designation,
                                    msg.type(), msg.mmsi(), msg.sog,
                                    msg.latitude, msg.longitude, miles);

                        }
                        break;
                    }
                    case 18: {
                        AISMessage18 msg;
                        if (msg.decode (e.rxPacket)) {
                            double distance = Utils::haversineDistance (mLat, mLng, msg.latitude, msg.longitude);
                            double miles = distance / METERS_PER_NAUTICAL_MILE;

                            printf2 ("RSSI: %.2x, Ch: %c, Type: %d, MMSI: %d, Speed: %.1f kts, Pos: %.5f,%.5f, Dist: %.1f NM\r\n",
                                    e.rxPacket.rssi (),
                                    AIS_CHANNELS[e.rxPacket.channel()].designation,
                                    msg.type(), msg.mmsi(), msg.sog,
                                    msg.latitude, msg.longitude, miles);

                        }
                        break;
                    }
                    default: {

                        printf2 (
                                "RSSI: %.2x, Ch: %c, Type: %d, RI: %d, MMSI: %d\r\n",
                                e.rxPacket.rssi (),
                                AIS_CHANNELS[e.rxPacket.channel()].designation,
                                e.rxPacket.messageType (),
                                e.rxPacket.repeatIndicator (),
                                e.rxPacket.mmsi ());

                        break;
                    }
                }


                mSentences.clear();
                RXPacket p(e.rxPacket);
                mEncoder.encode(p, mSentences);
                for (vector<string>::iterator i = mSentences.begin(); i != mSentences.end(); ++i) {
#ifdef MULTIPLEXED_OUTPUT
                    sprintf(__buff, "%s\r\n", i->c_str());
                    DataTerminal::instance().write("NMEA", __buff);
#else
                    DataTerminal::instance().write(i->c_str());
                    DataTerminal::instance().write("\r\n");
#endif
                }

                switch (e.rxPacket.messageType()) {
                    case 15:
                        // TODO: This is an interrogation. If we are a target, push an appropriate event into the queue
                        break;
                    case 20:
                        // TODO: This is a time slot reservation from a base station. Possibly use this information to augment CCA?
                        break;
                    case 22:
                        /*
                         * TODO: This is the frequency management message. If we support all the VHF channels in the specification
                         * (161.500Mhz to 162.025Mhz), we use this to switch our primary 2 channels where instructed.
                         */

                        /* 
                           Unfortunately, I can't find any authoritative documentation about the exact representation of
                           frequencies and/or channels in AIS message 22
                        */
                        
                        break;
                    case 23:
                        /*
                         * TODO: This is the group assignment message. Base stations can use this to configure our transmission interval.
                         */
                        break;
                }

            }
            else {
                ++mBadCRCCount;
            }
            break;
        }
        default:
            break;
    }

    //printf2("<- RXPacketProcessor::processEvent()\r\n");
}

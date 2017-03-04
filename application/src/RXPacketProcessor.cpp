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
    switch(e.type) {
    case GPS_FIX_EVENT: {
        mLat = e.gpsFix.lat;
        mLng = e.gpsFix.lng;
        break;
    }
    case CLOCK_EVENT: {
        if ( mLastDumpTime == 0 ) {
            mLastDumpTime = e.clock.utc;
        }
        else if ( e.clock.utc - mLastDumpTime >= 60 ) {
            mLastDumpTime = e.clock.utc;
            float yield = (float)mGoodCount / (float)(mGoodCount+mBadCRCCount+mInvalidCount);
            printf2("\r\n");
            printf2("[Yield: %dpct, Valid: %d, Wrong CRC: %d, Malformed: %d]\r\n", (int)(yield*100.0),
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
        LEDManager::instance().blink(LEDManager::GREEN_LED);

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
#ifdef OUTPUT_AIS_DEBUG            
                AISMessage123 msg;
                if (msg.decode (e.rxPacket)) {
                    double distance = Utils::haversineDistance (mLat, mLng, msg.latitude, msg.longitude);
                    double miles = distance / METERS_PER_NAUTICAL_MILE;

                    printf2 ("RSSI: %.2x, Ch: %c, Type: %d, MMSI: %d, Speed: %.1f kts, Pos: %.5f,%.5f, Dist: %.1f NM\r\n",
                             e.rxPacket.rssi(),
                             AIS_CHANNELS[e.rxPacket.channel()].designation,
                             msg.type(), msg.mmsi(), msg.sog,
                             msg.latitude, msg.longitude, miles);

                }
#endif
                break;
            }
            case 18: {
#ifdef OUTPUT_AIS_DEBUG            
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
#endif
                break;
            }
            case 15: {
                AISMessage15 msg;
                if ( msg.decode(e.rxPacket) ) {
#ifdef OUTPUT_AIS_DEBUG            
                    printf2 ("RSSI: %.2x, Ch: %c, Type: %d, MMSI: %d, Targets: [ {%d,%d} {%d,%d}, {%d,%d} ]\r\n",
                             e.rxPacket.rssi (),
                             AIS_CHANNELS[e.rxPacket.channel()].designation,
                             msg.type(), msg.mmsi(),
                             msg.targets[0].mmsi, msg.targets[0].messageType,
                             msg.targets[1].mmsi, msg.targets[1].messageType,
                             msg.targets[2].mmsi, msg.targets[2].messageType);
#endif
                    
                    // Make sure we actually can transmit something
                    if ( mStationData.magic != STATION_DATA_MAGIC )
                        break;
                    
                    // This is an interrogation. If we are a target, push an appropriate event into the queue

                    // It is possible that we are the target for more than one type of message (18 + 24)
                    for ( uint8_t i = 0; i < 3; ++i ) {
                        AISMessage15::InterrogationTarget &target = msg.targets[i];
                        if ( target.mmsi == mStationData.mmsi ) {
                            switch(target.messageType) {
                            case 18:
                            case 24: {
                                Event *ie = EventPool::instance().newEvent(INTERROGATION_EVENT);
                                ie->interrogation.channel = e.rxPacket.channel();
                                ie->interrogation.messageType = target.messageType;

                                printf2("Scheduling message %d in response to interrogation\r\n", ie->interrogation.messageType);
                                EventQueue::instance().push(ie);
                                break;
                            }
                            default:
                                // WTF?? Probably a misbehaved station
                                printf2("Ignoring malformed message 15 from MMSI %d\r\n", e.rxPacket.mmsi());
                                break;
                            }
                        }
                    }                
                
                }
                break;
            }
            default: {

#ifdef OUTPUT_AIS_DEBUG            
                printf2 (
                         "RSSI: %.2x, Ch: %c, Type: %d, RI: %d, MMSI: %d\r\n",
                         e.rxPacket.rssi (),
                         AIS_CHANNELS[e.rxPacket.channel()].designation,
                         e.rxPacket.messageType (),
                         e.rxPacket.repeatIndicator (),
                         e.rxPacket.mmsi ());

                break;
#endif
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


            // Special handling for specific messages that we care about
            switch (e.rxPacket.messageType()) {
            case 20:
                // TODO: This is a time slot reservation from a base station. Use this information to block those time slots.
                break;
            case 22:
                /*
                  TODO: 
                  This is the frequency management message. Since we support all of the upper VHF band (161.500 MHz - 162.025 MHz), 
                  we use this to switch our primary 2 channels where instructed. That said, I'm not completely clear on how channels
                  are described in this message. The spec references ITU-R M.1084 and allocates 12 bits for the channel number,
                  but whoever wrote this document did not care about comprehension :(
                  
                  https://www.itu.int/dms_pubrec/itu-r/rec/m/R-REC-M.1084-5-201203-I!!PDF-E.pdf
                */
                
                break;
            case 23:
                /*
                  TODO: This is the group assignment message. Base stations can use this to configure our transmission interval
                        or enforce silent periods as well. We should comply. 
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

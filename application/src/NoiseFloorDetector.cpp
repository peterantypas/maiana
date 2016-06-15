/*
 * NoiseFloorDetector.cpp
 *
 *  Created on: May 22, 2016
 *      Author: peter
 */

#include "NoiseFloorDetector.hpp"
#include "EventQueue.hpp"
#include <algorithm>
#include "printf2.h"
#include "AISChannels.h"

#define WINDOW_SIZE 10

NoiseFloorDetector &NoiseFloorDetector::instance()
{
    static NoiseFloorDetector __instance;
    return __instance;
}

NoiseFloorDetector::NoiseFloorDetector()
    : mUTC(0), mStartTime(0), mLastDumpTime(0)
{
    mSorted.reserve(WINDOW_SIZE);
    EventQueue::instance().addObserver(this, CLOCK_EVENT);
}

void NoiseFloorDetector::report(VHFChannel channel, uint8_t rssi)
{
    // If we don't have time yet, we certainly don't have fix so we can't be transmitting anyway, so no data collection
    if ( mUTC == 0 )
        return;

    if ( mData.find(channel) == mData.end() ) {
        ChannelReadings r;
        r.reserve(WINDOW_SIZE);
        mData[channel] = r;
    }

    ChannelReadings &window = mData[channel];
    processSample(window, rssi);
}

uint8_t NoiseFloorDetector::getNoiseFloor(VHFChannel channel)
{
    if ( mData.find(channel) == mData.end() )
        return 0xff;

    return medianValue(mData[channel]);
}


void NoiseFloorDetector::processEvent(const Event &e)
{
    if ( e.clock.utc == 0 ) {
        printf2("Starting RSSI sample collection\r\n");
        mStartTime = e.clock.utc;
        mLastDumpTime = mStartTime;
    }
    mUTC = e.clock.utc;

    if ( mUTC - mLastDumpTime >= 30 )
        dump();

    if ( mUTC - mStartTime > 180 ) {
        reset();
        mStartTime = mUTC;
    }
}

void NoiseFloorDetector::processSample(ChannelReadings &window, uint8_t rssi)
{
    while ( window.size() >= WINDOW_SIZE )
        window.pop_back();

    if ( window.empty() ) {
        Reading r;
        r.timestamp = mUTC;
        r.reading = rssi;
        window.push_back(r);
        return;
    }

    // Insert the reading at the start if it qualifies
    for ( ChannelReadings::iterator i = window.begin(); i != window.end(); ++i ) {
        if ( rssi <= i->reading ) {
            Reading r;
            r.timestamp = mUTC;
            r.reading = rssi;
            window.insert(window.begin(), r);
            break;
        }
    }
}

uint8_t NoiseFloorDetector::medianValue(ChannelReadings &window)
{
    if ( window.empty() )
        return 0xff;

    mSorted.clear();
    for ( ChannelReadings::iterator i = window.begin(); i != window.end(); ++i )
        mSorted.push_back(i->reading);

    sort(mSorted.begin(), mSorted.end());
    return mSorted[mSorted.size()/2];
}


void NoiseFloorDetector::dump()
{
    //printf2("Dumping RSSI stats:\r\n");
    for ( ChannelData::iterator cIt = mData.begin(); cIt != mData.end(); ++cIt ) {
        uint8_t value = medianValue(cIt->second);
        printf2("[Channel %d noise floor: 0x%.2x]\r\n", AIS_CHANNELS[cIt->first].itu, value);
    }

    mLastDumpTime = mUTC;
}

void NoiseFloorDetector::reset()
{
    mData.clear();
}




/*
 * NoiseFloorDetector.hpp
 *
 *  Created on: May 22, 2016
 *      Author: peter
 */

#ifndef NOISEFLOORDETECTOR_HPP_
#define NOISEFLOORDETECTOR_HPP_

#include "AISChannels.h"
#include "Events.hpp"
#include <map>

using namespace std;

class NoiseFloorDetector : public EventConsumer
{
public:
    static NoiseFloorDetector &instance();


    // Called directly by each receiver to report every RSSI reading at every SOTDMA slot, returns latest noise floor or 0xff if not enough data exists
    uint8_t report(VHFChannel channel, uint8_t rssi);

    void reset();

    // Returns the current noise floor of the channel, 0xff if unknown
    uint8_t getNoiseFloor(VHFChannel channelIndex);

    void processEvent(Event *e);
private:
    typedef struct {
        time_t timestamp;
        uint8_t reading;
    } Reading;

    typedef vector<Reading> ChannelReadings;
    typedef map<VHFChannel, ChannelReadings> ChannelData;

private:
    NoiseFloorDetector();
    uint8_t processSample(ChannelReadings &window, uint8_t rssi);
    uint8_t medianValue(ChannelReadings &window);
    void dump();
private:
    time_t          mUTC;
    time_t          mStartTime;
    time_t          mLastDumpTime;
    ChannelData     mData;
};

#endif /* NOISEFLOORDETECTOR_HPP_ */

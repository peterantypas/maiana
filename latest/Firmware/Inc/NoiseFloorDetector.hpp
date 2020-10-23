/*
  Copyright (c) 2016-2020 Peter Antypas

  This file is part of the MAIANA™ transponder firmware.

  The firmware is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>
*/


#ifndef NOISEFLOORDETECTOR_HPP_
#define NOISEFLOORDETECTOR_HPP_

#include "AISChannels.h"
#include "Events.hpp"
#include <map>


using namespace std;

/**
 * This class tracks the lowest RSSI values read over a 30 second period for each channel
 * and uses the median to establish the noise floor for Clear Channel Assessment
 */


class NoiseFloorDetector : public EventConsumer
{
public:
  static NoiseFloorDetector &instance();


  // Called directly by each receiver to report every RSSI reading at every SOTDMA slot
  void report(VHFChannel channel, uint8_t rssi);

  void reset();

  // Returns the current noise floor of the channel, 0xff if unknown
  uint8_t getNoiseFloor(VHFChannel channelIndex);

  void processEvent(const Event &e);

private:
  typedef struct
  {
    uint8_t reading;
  } Reading;

  // TODO: Use a circular buffer instead, no need for STL here
  typedef vector<Reading> ChannelReadings;
  typedef map<VHFChannel, ChannelReadings> ChannelData;

private:
  NoiseFloorDetector();
  void processSample(ChannelReadings &window, uint8_t rssi);
  uint8_t medianValue(ChannelReadings &window);
  //void dump();
private:
  uint32_t        mTicks;
  ChannelData     mData;
};


#endif /* NOISEFLOORDETECTOR_HPP_ */

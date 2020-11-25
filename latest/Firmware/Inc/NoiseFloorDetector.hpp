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

using namespace std;

class NoiseFloorDetector : public EventConsumer
{
public:
  static NoiseFloorDetector &instance();

  // Called directly by each receiver to report every RSSI reading at every SOTDMA slot
  void report(char channel, uint8_t rssi);

  void recalculate();

  // Returns the current noise floor of the channel, 0xff if unknown
  uint8_t getNoiseFloor(char channel);

  void processEvent(const Event &e);

private:

  //typedef vector<uint8_t> ChannelReadings;

  //ChannelReadings mChannelASamples;
  //ChannelReadings mChannelBSamples;

  uint8_t         mChannelACurrent;
  uint8_t         mChannelBCurrent;

  uint8_t         mAFloor;
  uint8_t         mBFloor;
private:
  NoiseFloorDetector();
  //void processSample(ChannelReadings &window, uint8_t rssi);
  //uint8_t medianValue(ChannelReadings &window);
  void dump();
private:
  uint32_t        mTicks;
};


#endif /* NOISEFLOORDETECTOR_HPP_ */

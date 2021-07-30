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


#ifndef RXPACKETPROCESSOR_HPP_
#define RXPACKETPROCESSOR_HPP_

#include "Events.hpp"
#include "NMEAEncoder.hpp"
#include <vector>
#include "Configuration.hpp"

class RXPacketProcessor : public EventConsumer
{
public:
  RXPacketProcessor ();
  virtual ~RXPacketProcessor ();

  void processEvent(const Event &e);

private:
  void ensureChannelIsTracked(VHFChannel ch);

private:
  class PacketStats
  {
  public:
    PacketStats()
    {
      good = 0;
      bad = 0;
      invalid = 0;
    }
    uint32_t good;
    uint32_t bad;
    uint32_t invalid;
  };

  NMEAEncoder mEncoder;
  std::vector<std::string> mSentences;
  StationData mStationData;
};

#endif /* RXPACKETPROCESSOR_HPP_ */

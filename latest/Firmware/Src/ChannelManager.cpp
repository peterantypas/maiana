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


#include "ChannelManager.hpp"
#include "EventQueue.hpp"


ChannelManager &ChannelManager::instance()
{
  static ChannelManager __instance;
  return __instance;
}

ChannelManager::ChannelManager()
: mChannelA(19), mChannelB(21)
{
  EventQueue::instance().addObserver(this, AIS_PACKET_EVENT);
}


ChannelManager::~ChannelManager()
{
  // TODO Auto-generated destructor stub
}

const ais_channel &ChannelManager::channelA()
{
  return AIS_CHANNELS[mChannelA];
}


const ais_channel &ChannelManager::channelB()
{
  return AIS_CHANNELS[mChannelB];
}

bool ChannelManager::channelsDetermined()
{
  // For now ...
  return true;
}

void ChannelManager::processEvent(const Event &)
{
  //AISPacketEvent *pe = static_cast<AISPacketEvent*>(e);
}


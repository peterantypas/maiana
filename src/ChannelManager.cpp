/*
 * ChannelManager.cpp
 *
 *  Created on: May 8, 2016
 *      Author: peter
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

void ChannelManager::processEvent(Event *)
{
    //AISPacketEvent *pe = static_cast<AISPacketEvent*>(e);
}


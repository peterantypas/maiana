/*
 * ChannelManager.hpp
 *
 *  Created on: May 8, 2016
 *      Author: peter
 */

#ifndef CHANNELMANAGER_HPP_
#define CHANNELMANAGER_HPP_

#include "AISChannels.h"
#include "Events.hpp"

class ChannelManager : public EventConsumer
{
public:
    static ChannelManager &instance();
    virtual ~ChannelManager();

    void processEvent(const Event &e);

    const ais_channel &channelA();
    const ais_channel &channelB();

    bool channelsDetermined();

private:
    ChannelManager();

    uint8_t mChannelA;
    uint8_t mChannelB;
};

#endif /* CHANNELMANAGER_HPP_ */

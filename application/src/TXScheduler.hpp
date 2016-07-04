/*
 * TXScheduler.hpp
 *
 *  Created on: Dec 12, 2015
 *      Author: peter
 */

#ifndef TXSCHEDULER_HPP_
#define TXSCHEDULER_HPP_

#include "Events.hpp"
#include <time.h>
#include "ObjectPool.hpp"
#include "AISChannels.h"
#include "EEPROM.hpp"


class TXScheduler : public EventConsumer
{
public:
    TXScheduler ();
    virtual
    ~TXScheduler ();

    void processEvent(const Event &event);
    void startTXTesting();
private:
    void scheduleTestPacket();
    time_t positionReportTimeInterval();
private:
    VHFChannel mPositionReportChannel;
    VHFChannel mStaticDataChannel;
    time_t mUTC;
    time_t mLast18Time;
    time_t mLast24Time;
    double mAvgSpeed;
    bool mTesting;
};

#endif /* TXSCHEDULER_HPP_ */

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
#include "Configuration.hpp"


class TXScheduler : public EventConsumer
{
public:
    static TXScheduler &instance();
    void init();
    void processEvent(const Event &event);
    void startTXTesting();
private:
    TXScheduler ();
    virtual
    ~TXScheduler ();
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
    StationData mStationData;
};

#endif /* TXSCHEDULER_HPP_ */

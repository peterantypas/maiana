/*
 * Event.hpp
 *
 *  Created on: Dec 7, 2015
 *      Author: peter
 */

#ifndef EVENTS_HPP_
#define EVENTS_HPP_

#include "EventTypes.h"
#include <time.h>
#include <cstring>
#include "NMEASentence.hpp"
#include <string>
#include "RXPacket.hpp"
#include "ObjectPool.hpp"


using namespace std;


/*
 * All events extend this class
 */
class Event
{
public:
    Event () {};
    virtual ~Event () {};
    virtual void prepare() {}
    virtual void clear() {}

    virtual EventType type() = 0;
};

/*
 * Event consumer abstract definition.
 */

class EventConsumer
{
public:
    virtual ~EventConsumer() {}
    virtual void processEvent(Event *event)=0;
};

class GPSNMEASentence : public Event
{
public:
    EventType type() {
        return GPS_NMEA_SENTENCE;
    }


    char mSentence[100];
};

class GPSFIXEvent: public Event
{
public:
    EventType type() {
        return GPS_FIX_EVENT;
    }

    time_t mUTC;
    double mLat;
    double mLng;
    double mSpeed;
    double mCOG;
};

class ClockEvent : public Event
{
public:
    EventType type() {
        return CLOCK_EVENT;
    }

    time_t mTime;
};

class AISPacketEvent: public Event
{
public:
    EventType type() {
        return AIS_PACKET_EVENT;
    }

    void prepare()
    {
        //mPacket = RXPacketPool::instance().newRXPacket();
        //mPacket->reset();
    }

    void clear()
    {
        RXPacketPool::instance().deleteRXPacket(mPacket);
        mPacket = NULL;
    }

    RXPacket *mPacket;
};

class AISPacketSentEvent : public Event
{
public:
    EventType type() {
        return PACKET_SENT_EVENT;
    }

    uint8_t mChannel;
    uint16_t mSize;
};


class DebugEvent: public Event
{
public:
    EventType type() {
        return DEBUG_EVENT;
    }

    char mBuffer[256];
};

class KeyPressEvent : public Event
{
public:
    EventType type() {
        return KEYPRESS_EVENT;
    }

    char key;
};

class EventPool
{
public:
    static EventPool &instance();

    void init();
    Event *newEvent(EventType type);
    void deleteEvent(Event *event);

private:
    ObjectPool<AISPacketEvent> *mAISPacketPool;
    ObjectPool<GPSNMEASentence> *mGPSNMEAPool;
    ObjectPool<GPSFIXEvent> *mGPSFixPool;
    ObjectPool<ClockEvent> *mClockPool;
    ObjectPool<AISPacketSentEvent> *mAISPacketSentPool;
    ObjectPool<DebugEvent> *mDebugEventPool;
    ObjectPool<KeyPressEvent> *mKeyPressPool;
};


#endif /* EVENTS_HPP_ */

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
    friend class EventPool;
public:
    Event()
        : mType(UNKNOWN_EVENT) {
    }

    Event (EventType type): mType(type) {
    }

    virtual ~Event () {
    }

    virtual void prepare() {
    }

    virtual void clear() {
    }

    virtual EventType type() {
        return mType;
    }
protected:
    EventType mType;
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
    GPSNMEASentence()
        : Event(GPS_NMEA_SENTENCE){
    }

    char mSentence[100];
};

class GPSFIXEvent: public Event
{
public:
    GPSFIXEvent()
        : Event(GPS_FIX_EVENT) {
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
    ClockEvent()
        : Event(CLOCK_EVENT) {
    }


    time_t mTime;
};

class AISPacketEvent: public Event
{
public:
    AISPacketEvent()
        : Event(AIS_PACKET_EVENT) {
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


class DebugEvent: public Event
{
public:
    DebugEvent()
        : Event(DEBUG_EVENT) {
    }

    char mBuffer[256];
};

class KeyPressEvent : public Event
{
public:
    KeyPressEvent()
        : Event(KEYPRESS_EVENT) {
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
    ObjectPool<Event> *mGenericPool;
    ObjectPool<AISPacketEvent> *mAISPacketPool;
    ObjectPool<GPSNMEASentence> *mGPSNMEAPool;
    ObjectPool<GPSFIXEvent> *mGPSFixPool;
    ObjectPool<ClockEvent> *mClockPool;
    ObjectPool<DebugEvent> *mDebugEventPool;
    ObjectPool<KeyPressEvent> *mKeyPressPool;
};


#endif /* EVENTS_HPP_ */

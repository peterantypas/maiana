/*
 * Event.cpp
 *
 *  Created on: Dec 7, 2015
 *      Author: peter
 */

#include "Events.hpp"
#include "printf2.h"


EventPool &EventPool::instance()
{
    static EventPool __instance;
    return __instance;
}

void EventPool::init()
{
    mAISPacketPool = new ObjectPool<AISPacketEvent>(20);
    mGPSNMEAPool = new ObjectPool<GPSNMEASentence>(20);
    mGPSFixPool = new ObjectPool<GPSFIXEvent>(10);
    mClockPool = new ObjectPool<ClockEvent>(10);
    mAISPacketSentPool = new ObjectPool<AISPacketSentEvent>(5);
    mDebugEventPool = new ObjectPool<DebugEvent>(2);
    mKeyPressPool = new ObjectPool<KeyPressEvent>(20);
}

Event *EventPool::newEvent(EventType type)
{
    Event *result = NULL;
    switch(type) {
        case GPS_NMEA_SENTENCE:
            result = mGPSNMEAPool->get();
            break;
        case GPS_FIX_EVENT:
            result = mGPSFixPool->get();
            break;
        case CLOCK_EVENT:
            result = mClockPool->get();
            break;
        case AIS_PACKET_EVENT: {
            result = mAISPacketPool->get();
            break;
        }
        case PACKET_SENT_EVENT: {
            result = mAISPacketSentPool->get();
            break;
        }
        case DEBUG_EVENT: {
            result = mDebugEventPool->get();
            break;
        }
        case KEYPRESS_EVENT: {
            result = mKeyPressPool->get();
            break;
        }
        default:
            result = NULL;
    }

    if ( result )
        result->prepare();

    return result;
}

void EventPool::deleteEvent(Event *event)
{
    event->clear();
    switch(event->type()) {
        case GPS_NMEA_SENTENCE: {
            GPSNMEASentence *e = static_cast<GPSNMEASentence*>(event);
            mGPSNMEAPool->put(e);
            break;
         }
        case GPS_FIX_EVENT: {
            GPSFIXEvent *e = static_cast<GPSFIXEvent*>(event);
            mGPSFixPool->put(e);
            break;
        }
        case CLOCK_EVENT:{
            ClockEvent *e = static_cast<ClockEvent*>(event);
            mClockPool->put(e);
            break;
        }
        case AIS_PACKET_EVENT: {
            AISPacketEvent *p = static_cast<AISPacketEvent*>(event);
            mAISPacketPool->put(p);
            break;
        }
        case PACKET_SENT_EVENT: {
            AISPacketSentEvent *p = static_cast<AISPacketSentEvent*>(event);
            mAISPacketSentPool->put(p);
            break;
        }
        case DEBUG_EVENT: {
            DebugEvent *e = static_cast<DebugEvent*>(event);
            mDebugEventPool->put(e);
            break;
        }
        case KEYPRESS_EVENT: {
            KeyPressEvent *e = static_cast<KeyPressEvent*>(event);
            mKeyPressPool->put(e);
            break;
        }
        default:
            break;
    }
}


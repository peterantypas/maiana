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
    mGenericPool = new ObjectPool<Event>(10);
}

Event *EventPool::newEvent(EventType type)
{
    Event *result = mGenericPool->get();
    result->type = type;
    return result;

}

void EventPool::deleteEvent(Event *event)
{
    mGenericPool->put(event);
}


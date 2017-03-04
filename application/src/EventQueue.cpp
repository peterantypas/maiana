/*
 * EventQueue.cpp
 *
 *  Created on: Dec 7, 2015
 *      Author: peter
 */

#include "EventQueue.hpp"
#include <cassert>
#include "printf2.h"
#include "LEDManager.hpp"
#include <stm32f30x.h>
#include "printf2.h"
#include "Utils.hpp"

EventQueue &EventQueue::instance()
{
    static EventQueue __instance;
    return __instance;
}

EventQueue::EventQueue()
{
    mQueue = new CircularQueue<Event*>(20);
}

void EventQueue::init()
{
}

void EventQueue::push(Event *event)
{
    assert(event);
    if ( !mQueue->push(event) ) {
        EventPool::instance().deleteEvent(event);
        printf2("EventQueue full!!!\r\n");
    }
}

void EventQueue::addObserver(EventConsumer *c, uint32_t eventMask)
{
    mConsumers[c] = eventMask;
}

void EventQueue::removeObserver(EventConsumer *c)
{
    map<EventConsumer*, uint32_t>::iterator i = mConsumers.find(c);
    if ( i == mConsumers.end() )
        return;

    mConsumers.erase(i);
}

void EventQueue::dispatch()
{
    Event *e = NULL;
    if (mQueue->pop(e)) {

        for ( map<EventConsumer*, uint32_t>::iterator c = mConsumers.begin(); c != mConsumers.end(); ++c ) {
            if ( c->second & e->type )
                c->first->processEvent(*e);
        }

        EventPool::instance().deleteEvent(e);
    }
}


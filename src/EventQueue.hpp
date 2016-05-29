/*
 * EventQueue.hpp
 *
 *  Created on: Dec 7, 2015
 *      Author: peter
 */

#ifndef EVENTQUEUE_HPP_
#define EVENTQUEUE_HPP_

#include <map>
#include "CircularQueue.hpp"
#include "Events.hpp"

using namespace std;

class EventQueue
{
public:
    static EventQueue &instance();

    void init();

    /*
     * Consumer registration
     */
    void addObserver(EventConsumer *c, uint32_t eventMask);

    /*
     * Consumer de-registration
     */
    void removeObserver(EventConsumer *c);

    /*
     * Interrupt code must push events to this queue to be processed by main() out of band.
     */
    void push(Event* event);

    /*
     * This method must be called repeatedly either by main() or some timer ISR.
     */
    void dispatch();
private:
    EventQueue();
    CircularQueue<Event*> *mQueue;
    map<EventConsumer *, uint32_t> mConsumers;
};

#endif /* EVENTQUEUE_HPP_ */

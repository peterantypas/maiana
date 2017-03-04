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

typedef struct {
    char sentence[88];
} NMEABuffer;

typedef struct {
    time_t utc;
    double lat;
    double lng;
    double speed;
    double cog;
} GPSFix;

typedef struct {
    time_t utc;
} ClockTick;

typedef struct {
    char buffer[88];
} DebugMessage;

typedef enum {
    OP_GET,
    OP_SET
} Operation;


typedef struct {
    Operation operation;
    char field[16];
    char value[64];
} Request;

typedef struct {
    bool success;
    char data[64];
} Response;

typedef struct {
    VHFChannel channel;
    uint8_t messageType;
} Interrogation;

class Event
{
public:
    EventType type;

    Event()
        : type(UNKNOWN_EVENT) {
    }

    union {
        NMEABuffer nmeaBuffer;
        GPSFix gpsFix;
        DebugMessage debugMessage;
        RXPacket rxPacket;
        ClockTick clock;
        Request request;
        Response response;
        Interrogation interrogation;
    };
};


/*
 * Event consumer abstract definition.
 */

class EventConsumer
{
public:
    virtual ~EventConsumer() {}
    virtual void processEvent(const Event &event)=0;
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
};


#endif /* EVENTS_HPP_ */

/*
 * EventTypes.h
 *
 *  Created on: Dec 7, 2015
 *      Author: peter
 */

#ifndef EVENTTYPES_H_
#define EVENTTYPES_H_


/*
 * Various events that flow through the system. Their identifiers form a bit mask for quick filtering,
 * which means we are limited to 32 distinct events.
 */

typedef enum {
    UNKNOWN_EVENT        =   0,         // Invalid, not a real event id
    GPS_NMEA_SENTENCE    =   1,         // A NMEA sentence was received from the GPS.
    GPS_FIX_EVENT        =   2,         // The GPS obtained a fix.
    CLOCK_EVENT          =   4,         // One pulse per second as triggered by GPS. This is a convenient 1 Hz "wall" clock, as it carries UTC.
    AIS_PACKET_EVENT     =   8,         // A packet was just decoded (not necessarily valid, must still be CRC checked, etc)
    PACKET_SENT_EVENT    =   16,
    DEBUG_EVENT          =   32,
    KEYPRESS_EVENT       =   64
    //GPS_ERROR_EVENT      =   64,        // A GPS failure
    //TRX_ERROR_EVENT      =   128         // A radio failure
}
EventType;



#endif /* EVENTTYPES_H_ */

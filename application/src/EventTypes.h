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
    UNKNOWN_EVENT        =   0x0000,         // Invalid, not a real event id
    GPS_NMEA_SENTENCE    =   0x0001,         // A NMEA sentence was received from the GPS.
    GPS_FIX_EVENT        =   0x0002,         // The GPS obtained a fix.
    CLOCK_EVENT          =   0x0004,         // One pulse per second as triggered by GPS. This is a convenient 1 Hz "wall" clock, as it carries UTC.
    AIS_PACKET_EVENT     =   0x0008,         // A packet was just decoded (not necessarily valid, must still be CRC checked, etc)
    INTERROGATION_EVENT  =   0x0010,         // Received AIS message 15
    DEBUG_EVENT          =   0x0020,
    KEYPRESS_EVENT       =   0x0040,
    REQUEST_EVENT        =   0x0080,
    RESPONSE_EVENT       =   0x00100,
    RESET_EVENT          =   0x00200
}
EventType;



#endif /* EVENTTYPES_H_ */

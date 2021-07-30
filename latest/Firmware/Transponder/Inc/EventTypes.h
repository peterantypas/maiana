/*
  Copyright (c) 2016-2020 Peter Antypas

  This file is part of the MAIANA™ transponder firmware.

  The firmware is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>
*/


#ifndef EVENTTYPES_H_
#define EVENTTYPES_H_


/*
 * Various events that flow through the system. Their identifiers form a bit mask for quick filtering,
 * which means we are limited to 32 distinct events.
 */

typedef enum {
    UNKNOWN_EVENT        =   0x00000,         // Invalid, not a real event id
    GPS_NMEA_SENTENCE    =   0x00001,         // A NMEA sentence was received from the GPS.
    GPS_FIX_EVENT        =   0x00002,         // The GPS obtained a fix.
    CLOCK_EVENT          =   0x00004,         // One event per second.
    AIS_PACKET_EVENT     =   0x00008,         // A packet was just decoded (not necessarily valid, must still be CRC checked, etc)
    INTERROGATION_EVENT  =   0x00010,         // Received AIS message 15
    DEBUG_EVENT          =   0x00020,         // Something to print to the output as a [DEBUG] message
    PROPR_NMEA_SENTENCE  =   0x00040,         // Proprietary NMEA sentence to be sent out
    DFU_EVENT            =   0x00080,         // Enter DFU mode
    COMMAND_EVENT        =   0x00100,         // An unparsed request (raw format)
    RSSI_SAMPLE_EVENT    =   0x00200          // An RSSI sample (very high frequency event)
}
EventType;



#endif /* EVENTTYPES_H_ */

// Local Variables:
// mode: c++
// End:

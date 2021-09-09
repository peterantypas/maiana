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
    AIS_NMEA_SENTENCE    =   0x00002,         // An AIS sentence received (!AIVDM), possible just a fragment
}
EventType;



#endif /* EVENTTYPES_H_ */

// Local Variables:
// mode: c++
// End:

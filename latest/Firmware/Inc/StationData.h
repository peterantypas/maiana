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


#ifndef STATIONDATA_H_
#define STATIONDATA_H_


#include <inttypes.h>
#include <time.h>
#include "AISChannels.h"

// The only types of craft we support:

typedef enum
{
  VESSEL_TYPE_UNSPECIFIED     = 0,
  VESSEL_TYPE_FISHING         = 30,
  VESSEL_TYPE_DIVING          = 34,
  VESSEL_TYPE_SAILING         = 36,
  VESSEL_TYPE_PLEASURECRAFT   = 37
} VesselType;

#define STATION_DATA_MAGIC 0xABADBABE


typedef struct
{
  uint32_t        magic;          // Magic value to indicate valid data (as opposed to erased FLASH/EEPROM)
  uint32_t        mmsi;           // Vessel MMSI (should really be 30 bit)
  char            name[21];       // Vessel name (all caps)
  char            callsign[8];    // Radio station call sign assigned with MMSI (if applicable). Default: empty
  uint8_t         len;            // Length in meters (default: 0)
  uint8_t         beam;           // Beam in meters   (default: 0)
  uint8_t         bowOffset;      // Distance between bow and GPS antenna (default: 0)
  uint8_t         portOffset;     // Distance between port side and GPS antenna (default: 0)
  VesselType      type;           // Vessel type as enumerated above (only this subset makes sense for class B)
} StationData;





#endif /* STATIONDATA_H_ */
// Local Variables:
// mode: c++
// End:

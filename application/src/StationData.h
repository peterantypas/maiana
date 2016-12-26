/*
 * StationData.h
 *
 *  Created on: May 30, 2016
 *      Author: peter
 */

#ifndef STATIONDATA_H_
#define STATIONDATA_H_


#include <inttypes.h>
#include <time.h>
#include "AISChannels.h"

// Station flags
#define     STATION_RX_ONLY         0x01
#define     STATION_PRIVACY_MODE    0x02

#define STATION_DATA_MAGIC 0xABADBABE

// For each special region designated in message 22, we store one of these
typedef struct {
    float       swLat;
    float       swLng;
    float       neLat;
    float       neLng;
    VHFChannel  channA;
    VHFChannel  channB;
    time_t      expiration;
} SpecialRegion;

struct StationData {
    uint32_t        magic;          // Magic value to indicate valid data (as opposed to erased FLASH garbage)
    uint32_t        mmsi;           // Vessel MMSI (should be 30 bit)
    char            name[21];       // Vessel name (all caps)
    char            callsign[8];    // Radio station call sign assigned with MMSI (if applicable)
    uint8_t         len;            // Length in meters (default: 0)
    uint8_t         beam;           // Beam in meters   (default: 0)
    uint8_t         flags;          // Reserved - 0 for now
    uint8_t         region_count;   // Number of special regions stored (0 by default)
    SpecialRegion   regions[5];     // Up to 5 special regions defined
};





#endif /* STATIONDATA_H_ */
// Local Variables:
// mode: c++
// End:

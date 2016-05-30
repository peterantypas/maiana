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


struct StationData {
    uint32_t        mmsi;           // Vessel MMSI (should be 30 bit)
    char            name[21];       // Vessel name (all caps)
    char            callsign[8];    // Radio station call sign assigned with MMSI
    uint8_t         len;            // Length in meters (default: 0)
    uint8_t         beam;           // Beam in meters   (default: 0)
    uint8_t         flags;          // Reserved - 0 for now
};


typedef struct {
    float       swLat;
    float       swLng;
    float       neLat;
    float       neLng;
    VHFChannel  channA;
    VHFChannel  channB;
    time_t      expiration;
} SpecialRegion;



#endif /* STATIONDATA_H_ */

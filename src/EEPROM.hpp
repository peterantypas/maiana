/*
 * EEPROM.hpp
 *
 *  Created on: May 26, 2016
 *      Author: peter
 */

#ifndef EEPROM_HPP_
#define EEPROM_HPP_

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

/*
static struct StationData __default_station_data = {
    987654321UL,
    "TEST STATION 01",
    "0N0000",
    0,
    0,
    0
};
*/


typedef struct {
    float       swLat;
    float       swLng;
    float       neLat;
    float       neLng;
    VHFChannel  channA;
    VHFChannel  channB;
    time_t      expiration;
} SpecialRegion;

#define EEPROM_STATION_ADDR         0x00    // A StationData structure starts here
#define EEPROM_REGION_CNT_ADDR      0x24    // Number of active special regions goes here (single byte)
#define EEPROM_REGION_ADDR          0x25    // Special region data array starts here

//static SpecialRegion* __special_regions = new SpecialRegion[3];

class EEPROM
{
public:
    static EEPROM &instance();

    void init();

    void writeStationData(const StationData &data);
    void readStationData(StationData &data);
    void writeRegionArray(SpecialRegion *regions, uint8_t numRegions);
    uint8_t readByte(uint8_t address);
    void writeByte(uint8_t address, uint8_t byte);

private:
    EEPROM();
};

#endif /* EEPROM_HPP_ */

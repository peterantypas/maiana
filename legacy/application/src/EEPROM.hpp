/*
 * EEPROM.hpp
 *
 *  Created on: May 26, 2016
 *      Author: peter
 */

#ifndef EEPROM_HPP_
#define EEPROM_HPP_

#include "StationData.h"


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
    StationData mData;
};

#endif /* EEPROM_HPP_ */

/*
 * Configuration.hpp
 *
 *  Created on: May 26, 2016
 *      Author: peter
 */

#ifndef CONFIGURATION_HPP_
#define CONFIGURATION_HPP_

// This singleton manages user-definable configuration data stored in Flash.

#include "StationData.h"


class Configuration
{
public:
    static Configuration &instance();

    void init();

    // Station data is separate from other configuration values and occupies a different address
    void writeStationData(const StationData &data);
    bool readStationData(StationData &data);

private:
    Configuration();
    void erasePage(uint32_t address);
    void lockFlash();
    void unlockFlash();
};

#endif /* CONFIGURATION_HPP_ */



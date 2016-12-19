/*
 * Configuration.cpp
 *
 *  Created on: May 26, 2016
 *      Author: peter
 */

#include <stm32f30x_flash.h>
#include <cstring>

#include "Configuration.hpp"
#include "printf2.h"
#include "globals.h"


Configuration &Configuration::instance()
{
    static Configuration __instance;
    return __instance;
}


Configuration::Configuration()
{
}

void Configuration::init()
{
}

void Configuration::erasePage(uint32_t address)
{
    FLASH_ErasePage(address);
    FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
}

void Configuration::writeStationData(const StationData &data)
{
    unlockFlash();
    erasePage(STATION_DATA_ADDRESS);    // 1K is enough
    const char *p = (const char*)&data;
    for ( size_t i = 0; i < sizeof data; i += 4, p += 4) {
        FLASH_ProgramWord(STATION_DATA_ADDRESS + i, *(uint32_t*)p);
        FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
    }

    lockFlash();
}

bool Configuration::readStationData(StationData &data)
{
    memcpy(&data, (const void*)STATION_DATA_ADDRESS, sizeof data);
    return data.magic == STATION_DATA_MAGIC;
}

void Configuration::unlockFlash()
{
    FLASH_Unlock();
    FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
}

void Configuration::lockFlash()
{
    FLASH_Lock();
    FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
}



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

#include <stm32l4xx_hal.h>
#include "Configuration.hpp"
#include "Utils.hpp"
#include "config.h"
#include "EventQueue.hpp"

#if 0
static StationData __THIS_STATION__ = {
    STATION_DATA_MAGIC,
    987654321,                // MMSI
    "NAUT",                   // Name
    "",                       // Call sign
    0,                        // Length overall
    0,                        // Beam
    0,                        // Bow offset
    0,                        // Port offset
    VESSEL_TYPE_UNSPECIFIED
};
#endif

static ConfigPage __page;

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
  bool cliBootMode = *(uint32_t*)BOOTMODE_ADDRESS == CLI_FLAG_MAGIC;
  if ( !cliBootMode )
    reportStationData();
}

void Configuration::reportStationData()
{
  StationData d;
  if ( !readStationData(d) )
    memset(&d, 0, sizeof d);

  Event *e = EventPool::instance().newEvent(PROPR_NMEA_SENTENCE);
  sprintf(e->nmeaBuffer.sentence,
      "$PAISTN,%lu,%s,%s,%d,%d,%d,%d,%d*",
      d.mmsi,
      d.name,
      d.callsign,
      d.type,
      d.len,
      d.beam,
      d.portOffset,
      d.bowOffset);

  Utils::completeNMEA(e->nmeaBuffer.sentence);
  EventQueue::instance().push(e);
}

bool Configuration::erasePage()
{
  uint32_t page = (CONFIGURATION_ADDRESS - FLASH_BASE) / FLASH_PAGE_SIZE;

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
  HAL_FLASH_Unlock();

  FLASH_EraseInitTypeDef erase;
  erase.TypeErase = FLASH_TYPEERASE_PAGES;
  erase.Banks     = FLASH_BANK_1;
  erase.Page      = page;
  erase.NbPages   = 1;

  uint32_t errPage;
  HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &errPage);
  if ( status != HAL_OK )
    {
      HAL_FLASH_Lock();
      return false;
    }

  HAL_FLASH_Lock();
  return true;
}

void Configuration::resetToDefaults()
{
  if ( erasePage() )
    reportStationData();
}

bool Configuration::writeStationData(const StationData &data)
{
  if ( !erasePage() )
    return false;

  memcpy(&__page.station, &data, sizeof data);
  if ( erasePage() )
    {
      bool success = writePage();
      reportStationData();
      return success;
    }
  else
    {
      return false;
    }
}

bool Configuration::writePage()
{
  uint32_t pageAddress = CONFIGURATION_ADDRESS;
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
  HAL_FLASH_Unlock();
  HAL_StatusTypeDef status = HAL_OK;
  for ( uint32_t dw = 0; dw < sizeof __page/8; ++dw )
    {
      status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, pageAddress + dw*8, __page.dw[dw]);
      if ( status != HAL_OK )
        break;
    }
  HAL_FLASH_Lock();

  return status == HAL_OK;
}

bool Configuration::readStationData(StationData &data)
{
  memcpy(&__page, (const void*)CONFIGURATION_ADDRESS, sizeof __page);
  memcpy(&data, &__page.station, sizeof data);
  return data.magic == STATION_DATA_MAGIC;
}




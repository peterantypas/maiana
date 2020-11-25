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
#include <cstring>
#include "Configuration.hpp"
#include "Utils.hpp"
#include "config.h"
#include "EventQueue.hpp"
#include "bsp.hpp"
#include <stdio.h>


#if 0

// This has no effect anymore, just left here for reference

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
  reportStationData();
}

void Configuration::reportStationData()
{
  StationData d;
  if ( !readStationData(d) )
    memset(&d, 0, sizeof d);

  Event *e = EventPool::instance().newEvent(PROPR_NMEA_SENTENCE);
  if ( !e )
    return;

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

void Configuration::resetToDefaults()
{
  if ( bsp_erase_station_data() )
    bsp_reboot();
}

bool Configuration::writeStationData(const StationData &data)
{
  if ( bsp_save_station_data(data) )
    {
      bsp_reboot();
      return true;
    }

  return false;
}

bool Configuration::readStationData(StationData &data)
{
  return bsp_read_station_data(data) && data.magic == STATION_DATA_MAGIC;
}




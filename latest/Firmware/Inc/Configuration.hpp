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


#ifndef CONFIGURATION_HPP_
#define CONFIGURATION_HPP_

// This singleton manages user-definable configuration data stored in Flash (or EEPROM).

#include "StationData.h"

// Defining this as a union of data fields or 32 double words, as the L4 expects flash writes to be 8 bytes long
typedef union
{
  StationData station;
  uint64_t dw[32];
} ConfigPage;

class Configuration
{
public:
  static Configuration &instance();

  void init();

  // Station data is separate from other configuration values and occupies a different address
  bool writeStationData(const StationData &data);
  bool readStationData(StationData &data);
  void resetToDefaults();
private:
  Configuration();
  bool erasePage();
  bool writePage();
  void reportStationData();
};

#endif /* CONFIGURATION_HPP_ */



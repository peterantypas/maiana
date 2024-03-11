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

#include "StationData.h"
#include "OTPData.h"
#include "config.h"
#include "ConfigFlags.h"



class Configuration
{
public:
  static Configuration &instance();

  void init();

  bool writeStationData(const StationData &data);
  bool readStationData(StationData &data);
  bool isStationDataProvisioned();
  void eraseStationData();

  void reportStationData();
  void reportOTPData();
  const OTPData *readOTP();
  bool writeOTP(const OTPData &data);
  void reportSystemData();
  void enableTX();
  void disableTX();
  bool isTXEnabled();

  bool isXOTrimmed();
  void setXOTrimValue(uint8_t value);
  uint8_t getXOTrimValue();
  void reportXOTrimValue();

  void factoryReset();
private:

  Configuration();

  uint32_t nextAvailableOTPSlot();
  const char *hwRev();
  const char *serNum();
  const char *mcuType();
  const char *breakoutType();
  bool isBootloaderPresent();
private:
  ConfigFlags mFlags = {0};
};

#endif /* CONFIGURATION_HPP_ */



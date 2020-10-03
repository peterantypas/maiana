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


#ifndef TXPOWERSETTINGS_H_
#define TXPOWERSETTINGS_H_


#define MAX_TX_LEVEL 33;
#define MIN_TX_LEVEL 24;

#include <inttypes.h>

/*
 * These settings were derived from conducted measurements using a calibrated HP8563E spectrum analyzer
 */

typedef enum {
    PWR_P33,
    PWR_P30,
    PWR_P27,
    PWR_P24
}
tx_power_level;

typedef struct {
    tx_power_level level;       // For convenience
    uint8_t pa_mode;            // PA Mode
    uint8_t pa_level;           // PA power level (native parameter)
    uint8_t pa_bias_clkduty;    // PA bias clock duty
}
pa_params;



static const pa_params POWER_TABLE[] = {
        {PWR_P33, 0x48, 0x12, 0x00},
        {PWR_P30, 0x48, 0x18, 0x00},
        {PWR_P27, 0x49, 0x55, 0x1a},
        {PWR_P24, 0x49, 0x50, 0x15}
};


#endif /* TXPOWERSETTINGS_H_ */

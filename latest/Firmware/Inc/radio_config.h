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


#ifndef RADIO_CONFIG_STUB_H_
#define RADIO_CONFIG_STUB_H_

#include <stdint.h>
#include "radio_config_ph_all_channels.h"
#include "radio_config_si4362.h"

static const uint8_t si4463_radio_configuration[] = SI4463_RADIO_CONFIGURATION_DATA_ARRAY;
static const uint8_t si4362_radio_configuration[] = SI4362_RADIO_CONFIGURATION_DATA_ARRAY;


#endif /* RADIO_CONFIG_H_ */

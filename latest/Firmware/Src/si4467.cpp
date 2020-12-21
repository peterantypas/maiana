/*
 * si4467.cpp
 *
 *  Created on: Dec 20, 2020
 *      Author: peter
 */

#include <stdint.h>
#include "radio_config_si4467.h"


static uint8_t __si_4467_cfg[] = RADIO_CONFIGURATION_DATA_ARRAY;

uint8_t* get_si4467_config_array()
{
  return __si_4467_cfg;
}

/*
 * si4463.cpp
 *
 *  Created on: Dec 20, 2020
 *      Author: peter
 */


#include <stdint.h>
#include "radio_config_Si4463_nonstd_preamble.h"


static uint8_t __si_4463_cfg[] = RADIO_CONFIGURATION_DATA_ARRAY;

uint8_t* get_si4463_config_array()
{
  return __si_4463_cfg;
}



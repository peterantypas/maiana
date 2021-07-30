/*
 * si4460.cpp
 *
 *  Created on: Mar 13, 2021
 *      Author: peter
 */


#include <stdint.h>
#include "radio_config_si4460.h"


static uint8_t __si_4460_cfg[] = RADIO_CONFIGURATION_DATA_ARRAY;

uint8_t* get_si4460_config_array()
{
  return __si_4460_cfg;
}

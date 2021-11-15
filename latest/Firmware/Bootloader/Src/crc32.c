/*
 * crc32.c
 *
 *  Created on: Aug 6, 2018
 *      Author: peter
 */

/*
 * Adapted from here:
 * https://stackoverflow.com/questions/36584265/using-cortex-m4-hardware-support-to-compute-crc32
 */

#include "crc32.h"
#include <stm32l4xx_hal.h>

extern CRC_HandleTypeDef hcrc;


uint32_t crc32(void *data, uint32_t bytes)
{
  uint32_t *p32 = data ;
  uint32_t crc, crc_reg ;

  __HAL_CRC_DR_RESET(&hcrc);


  while (bytes >= 4)
    {
      hcrc.Instance->DR = __RBIT(*p32++) ;
      bytes -= 4 ;
    }
  crc_reg = hcrc.Instance->DR ;
  crc = __RBIT(crc_reg) ;

  if (bytes > 0)
    {
      uint32_t bits = 8 * bytes ;
      uint32_t xtra = 32 - bits ;
      uint32_t mask = (1 << bits) - 1 ;

      hcrc.Instance->DR = crc_reg ;

      hcrc.Instance->DR = __RBIT((*p32 & mask) ^ crc) >> xtra ;
      crc = (crc >> bits) ^ __RBIT(hcrc.Instance->DR);
    }

  return ~(crc) ;
}

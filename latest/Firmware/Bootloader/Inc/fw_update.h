/*
 * fw_update.h
 *
 *  Created on: Aug 3, 2018
 *      Author: peter
 */

#ifndef FW_UPDATE_H_
#define FW_UPDATE_H_

#include "stm32l4xx_hal.h"

typedef enum {
  WAITING,
  TRANSFERRING
}
TransferState;

typedef struct {
  uint32_t size;
  uint32_t crc32;
} ImageHeader;

typedef struct {
  uint32_t magic;
  uint32_t size;
  uint32_t crc32;
  uint32_t flags;
} Metadata;


typedef struct {
  TransferState state;
  ImageHeader image;
  uint32_t pos;

  uint32_t bytes;
  uint32_t pages;
} FirmwareUpdate;

static FirmwareUpdate firmwareUpdate;

void dfu_init();

#endif /* FW_UPDATE_H_ */

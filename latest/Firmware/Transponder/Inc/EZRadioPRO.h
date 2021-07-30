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


#ifndef EZRADIOPRO_H_
#define EZRADIOPRO_H_

#include <inttypes.h>

#include "radio_config.h"

// API COMMANDS
typedef enum
{
  NOP                     = 0x00,
  PART_INFO               = 0x01,
  POWER_UP                = 0x02,
  FUNC_INFO               = 0x10,
  SET_PROPERTY            = 0x11,
  GET_PROPERTY            = 0x12,
  GPIO_PIN_CFG            = 0x13,
  GET_ADC_READING         = 0x14,
  FIFO_INFO               = 0x15,
  PACKET_INFO             = 0x16,
  IRCAL                   = 0x17,
  PROTOCOL_CFG            = 0x18,
  GET_INT_STATUS          = 0x20,
  GET_PH_STATUS           = 0x21,
  GET_MODEM_STATUS        = 0x22,
  GET_CHIP_STATUS         = 0x23,
  START_TX                = 0x31,
  START_RX                = 0x32,
  REQ_DEVICE_STATE        = 0x33,
  CHANGE_STATE            = 0x34,
  RX_HOP                  = 0x36,
  READ_CMD_BUFFER         = 0x44,
  FRR_A_READ              = 0x50,
  FRR_B_READ              = 0x51,
  FRR_C_READ              = 0x53,
  FRR_D_READ              = 0x57,
  WRITE_TX_FIFO           = 0x66,
  READ_RX_FIFO            = 0x77
} Command;

typedef struct
{
  uint8_t ChipRevision;
  uint8_t PartNumberH;
  uint8_t PartNumberL;
  uint8_t Build;
  uint8_t PartIDH;
  uint8_t PartIDL;
  uint8_t Customer;
  uint8_t ROMID;
} PART_INFO_REPLY;

typedef struct
{
  uint8_t BootOptions;
  uint8_t CrystalOptions;
  uint8_t CrystalFrequency3;
  uint8_t CrystalFrequency2;
  uint8_t CrystalFrequency1;
  uint8_t CrystalFrequency0;
} POWER_UP_PARAMS;

typedef struct
{
  uint8_t Group;
  uint8_t NumProperties;
  uint8_t StartProperty;
  uint8_t Data[11];
} SET_PROPERTY_PARAMS;

typedef struct
{
  uint8_t Group;
  uint8_t NumProperties;
  uint8_t StartProperty;
} GET_PROPERTY_REQUEST;

typedef struct
{
  uint8_t Data[16];
} GET_PROPERTY_REPLY;

typedef struct
{
  uint8_t GPIO0;
  uint8_t GPIO1;
  uint8_t GPIO2;
  uint8_t GPIO3;
  uint8_t NIRQ;
  uint8_t SDO;
  uint8_t GENCFG;
} GPIO_PIN_CFG_PARAMS;

typedef GPIO_PIN_CFG_PARAMS GPIO_PIN_CFG_REPLY;

typedef struct
{
  uint8_t Pending;
  uint8_t Current;
  uint8_t Error;
  //uint8_t CommandErrorCmdId;
} CHIP_STATUS_REPLY;

typedef struct
{
  uint8_t Pending;
  uint8_t Status;
  uint8_t CurrentRSSI;
  //uint8_t LatchedRSSI;
  //uint8_t Ant1RSSI;
  //uint8_t Ant2RSSI;
  //uint8_t AFCFreqOffH;
  //uint8_t AFCFreqOffL;
} MODEM_STATUS_REPLY;

typedef struct
{
  uint8_t INT_Pending;
  uint8_t INT_Status;
  uint8_t PH_Pending;
  uint8_t PH_Status;
  uint8_t MDM_Pending;
  uint8_t MDM_Status;
  uint8_t CHP_Pending;
  uint8_t CHP_Status;
} INT_STATUS_REPLY;


typedef struct
{
  uint8_t     channel;
  uint8_t     condition;
  uint16_t    tx_len;
  uint8_t     tx_delay;
  uint8_t     repeats;
} TX_OPTIONS;

typedef struct
{
  uint8_t     channel;
  uint8_t     condition;
  uint16_t    rx_len;
  uint8_t     next_state1;
  uint8_t     next_state2;
  uint8_t     next_state3;
} RX_OPTIONS;


typedef struct
{
  uint8_t     state;
  uint8_t     channel;
}
DEVICE_STATE;


#endif /* EZRADIOPRO_H_ */

// Local Variables:
// mode: c++
// End:

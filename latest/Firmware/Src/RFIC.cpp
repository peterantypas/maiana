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


#include "RFIC.hpp"
#include "radio_config.h"
#include "Utils.hpp"
#include "EZRadioPRO.h"
#include <cstring>
#include "printf_serial.h"
#include "bsp.hpp"

RFIC::RFIC(GPIO_TypeDef *sdnPort,
    uint32_t sdnPin,
    GPIO_TypeDef *csPort,
    uint32_t csPin,
    GPIO_TypeDef *dataPort,
    uint32_t dataPin,
    GPIO_TypeDef *clockPort,
    uint32_t clockPin)
{
  mSDNP = sdnPort;
  mCSPort = csPort;
  mDataPort = dataPort;
  mClockGPIO = clockPort;

  mSDNPin = sdnPin;
  mCSPin = csPin;
  mDataPin = dataPin;
  mClockPin = clockPin;

  //mRSSIAdjustment = 0;
  mSPIBusy = false;

  if ( !isInitialized() )
    powerOnReset();

}

RFIC::~RFIC()
{
}

inline void RFIC::spiOn()
{
  HAL_GPIO_WritePin(mCSPort, mCSPin, GPIO_PIN_RESET);
}

inline void RFIC::spiOff()
{
  HAL_GPIO_WritePin(mCSPort, mCSPin, GPIO_PIN_SET);
}

bool RFIC::sendCmd(uint8_t cmd, void* params, uint8_t paramLen, void* result, uint8_t resultLen)
{
  mSPIBusy = true;

  spiOn();

  bsp_tx_spi_byte(cmd);

  uint8_t *b = (uint8_t*) params;
  for ( int i = 0; i < paramLen; ++i )
    {
      bsp_tx_spi_byte(b[i]);
    }
  spiOff();

  while ( readSPIResponse(result, resultLen) == false)
    ;

  mSPIBusy = false;
  return true;
}

// This is borrowed from the dAISy project. Thank you Adrian :)
bool RFIC::readSPIResponse(void *data, uint8_t length)
{
  spiOn();
  bsp_tx_spi_byte(0x44);
  if ( bsp_tx_spi_byte(0) != 0xff )
    {
      spiOff();
      return false;
    }

  uint8_t* b = (uint8_t*) data;
  uint8_t i = 0;
  while (i < length)
    {
      b[i] = bsp_tx_spi_byte(0);
      ++i;
    }

  spiOff();
  return true;
}

void RFIC::configure()
{
  uint8_t radio_configuration[] = RADIO_CONFIGURATION_DATA_ARRAY;
  uint8_t *cfg = radio_configuration;
  while (*cfg)
    {                              // configuration array stops with 0
      uint8_t count = (*cfg++) - 1;           // 1st byte: number of bytes, incl. command
      uint8_t cmd = *cfg++;                   // 2nd byte: command
      sendCmd(cmd, cfg, count, NULL, 0);      // send bytes to chip
      cfg += count;                           // point at next line
    }
}

bool RFIC::isInitialized()
{
  HAL_GPIO_WritePin(mSDNP, mSDNPin, GPIO_PIN_SET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(mSDNP, mSDNPin, GPIO_PIN_RESET);
  HAL_Delay(100);

  //DBG("Checking RF chip status\r\n");
  CHIP_STATUS_REPLY chip_status;
  memset(&chip_status, 0, sizeof chip_status);
  sendCmd(GET_CHIP_STATUS, NULL, 0, &chip_status, sizeof chip_status);
  //DBG("Chip status: 0x%.2x\r\n", chip_status.Current);
  if ( chip_status.Current & 0x08 )
    {
      return false;
    }
  else
    {
      return true;
  }
}

void RFIC::powerOnReset()
{
  //DBG("Performing Power On Reset\r\n");

  // Pull SDN high to shut down the IC
  HAL_GPIO_WritePin(mSDNP, mSDNPin, GPIO_PIN_SET);

  // Wait at least 10 microseconds
  HAL_Delay(1);

  // Pull SDN low and poll the status of GPIO1
  HAL_GPIO_WritePin(mSDNP, mSDNPin, GPIO_PIN_RESET);

  //DBG("Waiting for GPIO1\r\n");
  while ( HAL_GPIO_ReadPin(mDataPort, mDataPin) == GPIO_PIN_RESET )
    ;

  // We're done!
  //DBG("Radio Ready!\r\n");
}

uint8_t RFIC::readRSSI()
{
  MODEM_STATUS_REPLY s;
  if ( sendCmd(GET_MODEM_STATUS, NULL, 0, &s, sizeof s) )
    return s.CurrentRSSI;
  else
    return 0;
}

bool RFIC::checkStatus()
{
  DEVICE_STATE s;
  sendCmd(REQ_DEVICE_STATE, NULL, 0, &s, sizeof s);
  if ( s.state != 8 && s.state != 7 )
    return false;
  else
    return true;
}






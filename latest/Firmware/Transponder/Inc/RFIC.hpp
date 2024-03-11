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


#ifndef RFIC_HPP_
#define RFIC_HPP_

#include <inttypes.h>
#include <stm32l4xx_hal.h>


typedef enum
{
  BIT_STATE_PREAMBLE_SYNC,
  BIT_STATE_IN_PACKET
} BitState;

class RFIC
{
public:
  RFIC(GPIO_TypeDef *sdnPort,
       uint32_t sdnPin,
       GPIO_TypeDef *csPort,
       uint32_t csPin,
       GPIO_TypeDef *dataPort,
       uint32_t dataPin,
       GPIO_TypeDef *clockPort,
       uint32_t clockPin,
       int chipId);

  virtual ~RFIC();

  void setRSSIAdjustment(short rssiAdj);
  uint16_t partNumber();
  bool isResponsive();

  void setXOTrimValue(uint8_t value);
protected:
  virtual void configure();
  bool sendCmd(uint8_t cmd, void* params, uint8_t paramLen, void* result, uint8_t resultLen);
  bool sendCmdNoWait(uint8_t cmd, void* params, uint8_t paramLen);
  bool isInitialized();
  bool powerOnReset();
  bool isReceiving();
  uint8_t readRSSI();
  bool checkStatus();
  virtual void configureGPIOsForRX() = 0;
private:
  bool readSPIResponse(void *data, uint8_t len);
  inline void spiOn();
  inline void spiOff();
protected:
  GPIO_TypeDef        *mSDNP;            // The MCU GPIO assigned to SDN for this IC (GPIOA, GPIOB or GPIOC)
  GPIO_TypeDef        *mCSPort;          // The MCU GPIO assigned to CS for this IC
  GPIO_TypeDef        *mDataPort;        // The MCU GPIO assigned to the RX/TX data pin of the IC
  GPIO_TypeDef        *mClockGPIO;       // The MCU GPIO assigned to the RX/TX clock of the IC
  uint32_t            mSDNPin;
  uint32_t            mCSPin;
  uint32_t            mDataPin;
  uint32_t            mClockPin;
  uint8_t             mLastNRZIBit;
  BitState            mBitState;
  uint32_t            mChipID;
  bool                mCTSPending = false;
  uint16_t            mPartNumber;
  bool                mPORSuccess = false;
};

#endif /* RFIC_HPP_ */

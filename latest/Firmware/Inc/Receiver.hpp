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


#ifndef RECEIVER_HPP_
#define RECEIVER_HPP_

#include "RXPacket.hpp"
#include "RadioState.hpp"
#include "RFIC.hpp"
#include "AISChannels.h"



class Receiver : public RFIC
{
public:
  Receiver(GPIO_TypeDef *sdnPort,
          uint32_t sdnPin,
          GPIO_TypeDef *csPort,
          uint32_t csPin,
          GPIO_TypeDef *dataPort,
          uint32_t dataPin,
          GPIO_TypeDef *clockPort,
          uint32_t clockPin,
          int chipId);

  virtual ~Receiver();

  bool init();
  VHFChannel channel();
  virtual void startReceiving(VHFChannel channel, bool reconfigGPIOs);
  virtual void onBitClock();
  virtual void timeSlotStarted(uint32_t slot);
  void switchToChannel(VHFChannel channel);
protected:
  typedef enum
  {
    NO_ACTION,
    RESTART_RX,
    RETRIEVE_RSSI
  } Action;

  void startListening(VHFChannel channel, bool reconfigGPIOs);
  bool addBit(uint8_t bit);
  void resetBitScanner();
  uint8_t reportRSSI();
  void pushPacket();
  Action processNRZIBit(uint8_t level);
  virtual void configureGPIOsForRX();
protected:
  RXPacket *mRXPacket = nullptr;
  uint16_t mBitWindow;
  uint8_t mLastNRZIBit;
  uint32_t mBitCount;
  uint32_t mOneBitCount;
  BitState mBitState;
  uint8_t mRXByte;
  VHFChannel mChannel;
  int mSlotBitNumber;
  VHFChannel mNextChannel;
  uint32_t mTimeSlot = 0xffffffff;
};

#endif /* RECEIVER_HPP_ */

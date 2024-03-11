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


#ifndef TRANSCEIVER_HPP_
#define TRANSCEIVER_HPP_

#include "Receiver.hpp"
#include "TXPacket.hpp"
#include "Events.hpp"
#include <map>

class Transceiver : public Receiver, public EventConsumer
{
public:
  Transceiver(GPIO_TypeDef *sdnPort,
              uint32_t sdnPin,
              GPIO_TypeDef *csPort,
              uint32_t csPin,
              GPIO_TypeDef *dataPort,
              uint32_t dataPin,
              GPIO_TypeDef *clockPort,
              uint32_t clockPin,
              int chipId);


  void onBitClock();
  void timeSlotStarted(uint32_t slot);
  void assignTXPacket(TXPacket *p);
  TXPacket *assignedTXPacket();
  void startReceiving(VHFChannel channel, bool reconfigGPIOs);
  void transmitCW(VHFChannel channel);
  void processEvent(const Event &);

protected:
  void configure();
  virtual void configureGPIOsForRX();
private:
  void startTransmitting();
  void configureGPIOsForTX();
  void setTXPower(const pa_params &params);
  void reportTXEvent();
private:
  TXPacket    *mTXPacket;
  time_t      mUTC;
  time_t      mLastTXTime;
  //map<VHFChannel, uint8_t> mNoiseFloorCache;
};

#endif /* TRANSCEIVER_HPP_ */

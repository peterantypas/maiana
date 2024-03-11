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


#ifndef RADIOMANAGER_HPP_
#define RADIOMANAGER_HPP_

#include "Receiver.hpp"
#include "Transceiver.hpp"
#include "GPS.hpp"
#include "TXPacket.hpp"
#include "CircularQueue.hpp"
#include "EventQueue.hpp"
#include "AISChannels.h"


class RadioManager : public GPSDelegate, EventConsumer
{
public:
  static RadioManager &instance();

  void init();
  void start();
  void stop();
  void onBitClock(uint8_t ic);
  void timeSlotStarted(uint32_t slotNumber);

  void scheduleTransmission(TXPacket *p);
  bool initialized();

  void sendTestPacketNow(TXPacket *p);

  void processEvent(const Event &e);
  void transmitCW(VHFChannel channel);
  void stopTX();

  VHFChannel alternateChannel(VHFChannel channel);

  void setXOTrimValue(uint8_t value);

private:
  RadioManager();
  void spiOff();
  void configureInterrupts();
  void reportError(int chipId);
private:
  Transceiver *mTransceiverIC;
  Receiver *mReceiverIC;
  bool mInitializing;
  time_t mUTC = 0;
  time_t mStartTime = 0;

  CircularQueue<TXPacket*>  mTXQueue;
};

#endif /* RADIOMANAGER_HPP_ */

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


#include "Receiver.hpp"
#include "EZRadioPRO.h"
#include "Events.hpp"
#include "EventQueue.hpp"
#include "NoiseFloorDetector.hpp"
#include "bsp.hpp"

Receiver::Receiver(GPIO_TypeDef *sdnPort, uint32_t sdnPin, GPIO_TypeDef *csPort, uint32_t csPin,
    GPIO_TypeDef *dataPort, uint32_t dataPin,
    GPIO_TypeDef *clockPort, uint32_t clockPin)
: RFIC(sdnPort, sdnPin, csPort, csPin, dataPort, dataPin, clockPort, clockPin)
{
  mSlotBitNumber = 0xffff;
  mSwitchAtNextSlot = false;
  mOneBitCount = 0;
  mChannel = CH_88;
  mBitCount = 0;
  mBitState = BIT_STATE_PREAMBLE_SYNC;
  mLastNRZIBit=0x00;
  mSwitchToChannel = mChannel;
  mRXByte = 0;
  mBitWindow = 0;
}

Receiver::~Receiver()
{
}

VHFChannel Receiver::channel()
{
  return mChannel;
}

bool Receiver::init()
{
  //DBG("Configuring IC\r\n");
  configure();
  resetBitScanner();
  return true;
}

void Receiver::startReceiving(VHFChannel channel)
{
  mChannel = channel;
  startListening(mChannel);
  resetBitScanner();
}

void Receiver::switchToChannel(VHFChannel channel)
{
  mSwitchAtNextSlot = true;
  mSwitchToChannel = channel;
}

void Receiver::startListening(VHFChannel channel)
{
  configureGPIOsForRX();

  mChannel = channel;
  RX_OPTIONS options;
  options.channel = AIS_CHANNELS[channel].ordinal;
  options.condition = 0;
  options.rx_len = 0;
  options.next_state1 = 0;
  options.next_state2 = 0;
  options.next_state3 = 0;

  sendCmd (START_RX, &options, sizeof options, NULL, 0);
}

void Receiver::resetBitScanner()
{
  mBitWindow = 0;
  mBitCount = 0;
  mOneBitCount = 0;
  mLastNRZIBit = 0xff;
  mRXByte = 0;
  mBitState = BIT_STATE_PREAMBLE_SYNC;

  mRXPacket.reset();
}

void Receiver::onBitClock()
{
  // Don't waste time processing bits when the transceiver is transmitting
  if ( gRadioState == RADIO_TRANSMITTING )
    return;


  uint8_t bit = HAL_GPIO_ReadPin(mDataPort, mDataPin);
  processNRZIBit(bit);
#if 1
  if ( mSlotBitNumber != 0xffff && mSlotBitNumber++ == CCA_SLOT_BIT - 1 )
    {
      uint8_t rssi = reportRSSI();
      mRXPacket.setRSSI(rssi);
    }
#else
  ++mSlotBitNumber;
#endif
}

void Receiver::timeSlotStarted(uint32_t slot)
{
  // This should never be called while transmitting. Transmissions start after the slot boundary and end before the end of it.
  //assert(gRadioState == RADIO_RECEIVING);
  //if ( gRadioState != RADIO_RECEIVING )
  //DBG("    **** WTF??? Transmitting past slot boundary? **** \r\n");

  mSlotBitNumber = 0;
  if ( mBitState == BIT_STATE_IN_PACKET )
    return;

  mRXPacket.setSlot(slot);
  if ( mSwitchAtNextSlot )
    {
      mSwitchAtNextSlot = false;
      startReceiving(mSwitchToChannel);
    }
}

void Receiver::processNRZIBit(uint8_t bit)
{
  if ( mLastNRZIBit == 0xff )
    {
      mLastNRZIBit = bit;
      return;
    }

  uint8_t decodedBit = !(mLastNRZIBit ^ bit);

  switch (mBitState) {
  case BIT_STATE_PREAMBLE_SYNC:
    {
      mLastNRZIBit = bit;
      mBitWindow <<= 1;
      mBitWindow |= decodedBit;

      /*
       * By checking for the last few training bits plus the HDLC start flag,
       * we gain enough confidence that this is not random noise.
       */
      if ( mBitWindow == 0b1010101001111110 || mBitWindow == 0b0101010101111110 )
        {
          mBitState = BIT_STATE_IN_PACKET;
          mRXPacket.setChannel(mChannel);
        }

      break;
    }
  case BIT_STATE_IN_PACKET:
    {
      if ( mRXPacket.size() >= MAX_AIS_RX_PACKET_SIZE )
        {
          // Start over
          startReceiving(mChannel);

          return;
        }

      if ( mOneBitCount >= 7 )
        {
          // Bad packet!
          startReceiving(mChannel);
          return;
        }

      mLastNRZIBit = bit;
      mBitWindow <<= 1;
      mBitWindow |= decodedBit;



      if ( (mBitWindow & 0x00ff) == 0x7E )
        {
          mBitState = BIT_STATE_PREAMBLE_SYNC;
          pushPacket();
          startReceiving(mChannel);
        }
      else
        {
          addBit(decodedBit);
        }

      break;
    }
  }

}


bool Receiver::addBit(uint8_t bit)
{
  bool result = true;
  if ( bit )
    {
      ++mOneBitCount;
    }
  else
    {
      // Don't put stuffed bits into the packet
      if ( mOneBitCount == 5 )
        result = false;

      mOneBitCount = 0;
    }

  if ( result )
    {
      mRXByte <<= 1;
      mRXByte |= bit;
      ++mBitCount;
    }

  if ( mBitCount == 8 )
    {
      // Commit to the packet!
      mRXPacket.addByte(mRXByte);
      mBitCount = 0;
      mRXByte = 0;
    }

  return result;
}

void Receiver::pushPacket()
{
#ifndef TX_TEST_MODE
  Event p(AIS_PACKET_EVENT);
  p.rxPacket = mRXPacket;
  mRXPacket.reset();
  EventQueue::instance().push(p);
#else
  mRXPacket.reset();
#endif
}

uint8_t Receiver::reportRSSI()
{
  uint8_t rssi = readRSSI();

#if 0
  Event e(RSSI_SAMPLE_EVENT);
  e.rssiSample.channel = mChannel;
  e.rssiSample.rssi = rssi;
  EventQueue::instance().push(e);
#endif


  char channel = AIS_CHANNELS[mChannel].designation;
  NoiseFloorDetector::instance().report(channel, rssi);

  return rssi;
}

void Receiver::configureGPIOsForRX()
{
  /*
   * Configure radio GPIOs for RX:
   * GPIO 0: Don't care
   * GPIO 1: RX_DATA
   * GPIO 2: Don't care
   * GPIO 3: RX_TX_DATA_CLK
   * NIRQ  : SYNC_WORD_DETECT
   */

  GPIO_PIN_CFG_PARAMS gpiocfg;
  gpiocfg.GPIO0 = 0x00;       // No change
  gpiocfg.GPIO1 = 0x14;       // RX data bits
  gpiocfg.GPIO2 = 0x00;       // No change
  gpiocfg.GPIO3 = 0x1F;       // RX/TX data clock
  gpiocfg.NIRQ  = 0x1A;       // Sync word detect
  gpiocfg.SDO   = 0x00;       // No change
  gpiocfg.GENCFG = 0x00;      // No change
  sendCmd(GPIO_PIN_CFG, &gpiocfg, sizeof gpiocfg, &gpiocfg, sizeof gpiocfg);

  bsp_set_rx_mode();
}



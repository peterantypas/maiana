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
  mRXPacket = EventPool::instance().newRXPacket();
  ASSERT_VALID_PTR(mRXPacket);
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
  //configureGPIOsForRX();

  return true;
}

void Receiver::startReceiving(VHFChannel channel, bool reconfigGPIOs)
{
  mChannel = channel;
  startListening(mChannel, reconfigGPIOs);
  resetBitScanner();
}

void Receiver::switchToChannel(VHFChannel channel)
{
  mSwitchAtNextSlot = true;
  mSwitchToChannel = channel;
}

// TODO: This is a really, really long operation - over 320us !!!
void Receiver::startListening(VHFChannel channel, bool reconfigGPIOs)
{
  if ( reconfigGPIOs )
    {
      // This takes about 140us
      configureGPIOsForRX();
    }

  // This takes 180us
  mChannel = channel;
  RX_OPTIONS options;
  options.channel = AIS_CHANNELS[channel].ordinal;
  options.condition = 0;
  options.rx_len = 0;
  options.next_state1 = 0;
  options.next_state2 = 0;
  options.next_state3 = 0;

  /**
   * This can take up to 220us, that's 3 bit clocks!!!
   */
  //bsp_signal_high();
  sendCmd (START_RX, &options, sizeof options, NULL, 0);
  //bsp_signal_low();
}

void Receiver::resetBitScanner()
{
  mBitWindow = 0;
  mBitCount = 0;
  mOneBitCount = 0;
  mLastNRZIBit = 0xff;
  mRXByte = 0;
  mBitState = BIT_STATE_PREAMBLE_SYNC;

  mRXPacket->reset();
}

/*
 * TODO: Under a worst case scenario, this interrupt service method
 * can take up to 320us to complete (that's 4 clock bits!!!)
 *
 * Re-architecting will be necessary to resolve this, and it will almost certainly
 * require ditching FreeRTOS in favor of "bare metal".
 */

void Receiver::onBitClock()
{
  // Don't waste time processing bits when the transceiver is transmitting
  if ( gRadioState == RADIO_TRANSMITTING )
    return;

  //bsp_signal_high();

  uint8_t bit = HAL_GPIO_ReadPin(mDataPort, mDataPin);
  processNRZIBit(bit);
  if ( mSlotBitNumber != 0xffff && mSlotBitNumber++ == CCA_SLOT_BIT - 1 )
    {
      uint8_t rssi = reportRSSI();
      mRXPacket->setRSSI(rssi);
    }
  //bsp_signal_low();
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

  mRXPacket->setSlot(slot);
  if ( mSwitchAtNextSlot )
    {
      mSwitchAtNextSlot = false;
      startReceiving(mSwitchToChannel, false);
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
          mRXPacket->setChannel(mChannel);
        }

      break;
    }
  case BIT_STATE_IN_PACKET:
    {
      if ( mRXPacket->size() >= MAX_AIS_RX_PACKET_SIZE )
        {
          // Start over
          startReceiving(mChannel, false);

          return;
        }

      if ( mOneBitCount >= 7 )
        {
          // Bad packet!
          startReceiving(mChannel, false);
          return;
        }

      mLastNRZIBit = bit;
      mBitWindow <<= 1;
      mBitWindow |= decodedBit;



      if ( (mBitWindow & 0x00ff) == 0x7E )
        {
          mBitState = BIT_STATE_PREAMBLE_SYNC;
          pushPacket();
          startReceiving(mChannel, false);
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
      mRXPacket->addByte(mRXByte);
      mBitCount = 0;
      mRXByte = 0;
    }

  return result;
}

void Receiver::pushPacket()
{
  Event *p = EventPool::instance().newEvent(AIS_PACKET_EVENT);
  RXPacket *currPacket = mRXPacket;
  mRXPacket = EventPool::instance().newRXPacket();
  ASSERT_VALID_PTR(mRXPacket);

  if ( p )
    {
      //bsp_signal_high();
      p->rxPacket = currPacket;
      EventQueue::instance().push(p);
      //bsp_signal_low();
    }

  mRXPacket->reset();
}

uint8_t Receiver::reportRSSI()
{
  uint8_t rssi = readRSSI();
  char channel = AIS_CHANNELS[mChannel].designation;
  NoiseFloorDetector::instance().report(channel, rssi);
  return rssi;
}

void Receiver::configureGPIOsForRX()
{
  GPIO_PIN_CFG_PARAMS gpiocfg;
  gpiocfg.GPIO0 = 0x00;       // No change
  gpiocfg.GPIO1 = 0x14;       // RX data bits
  gpiocfg.GPIO2 = 0x00;       // No change
  gpiocfg.GPIO3 = 0x1F;       // RX/TX data clock
  gpiocfg.NIRQ  = 0x00;       // Nothing
  gpiocfg.SDO   = 0x00;       // No change
  gpiocfg.GENCFG = 0x00;      // No change
  sendCmd(GPIO_PIN_CFG, &gpiocfg, sizeof gpiocfg, &gpiocfg, sizeof gpiocfg);
}



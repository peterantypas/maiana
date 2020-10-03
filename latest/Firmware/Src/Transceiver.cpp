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


#include "Transceiver.hpp"
#include "NoiseFloorDetector.hpp"
#include "EventQueue.hpp"
#include "Events.hpp"
#include "EZRadioPRO.h"
#include "AISChannels.h"
#include "bsp.hpp"


Transceiver::Transceiver(GPIO_TypeDef *sdnPort, uint32_t sdnPin, GPIO_TypeDef *csPort,
    uint32_t csPin, GPIO_TypeDef *dataPort, uint32_t dataPin,
    GPIO_TypeDef *clockPort, uint32_t clockPin)
: Receiver(sdnPort, sdnPin, csPort, csPin, dataPort, dataPin, clockPort, clockPin)
{
  mTXPacket = NULL;
  EventQueue::instance().addObserver(this, CLOCK_EVENT);
  mUTC = 0;
  mLastTXTime = 0;
  mChannel = CH_87;
}

void Transceiver::configure()
{
  Receiver::configure();

  // Anything transmitter specific goes here
  SET_PROPERTY_PARAMS p;

  p.Group = 0x20;
  p.NumProperties = 1;
  p.StartProperty = 0x00;
  p.Data[0] = 0x20 | 0x08 | 0x03; // Synchronous direct mode from GPIO 1 with 2GFSK modulation
  sendCmd(SET_PROPERTY, &p, 4, NULL, 0);

  /**
   * We need maximum digital ramp control to reduce spurs. It's only about 200us, which
   * is less than the ramp-up bits in the TX packet, but it sure helped!
   */
  p.Group = 0x22;
  p.NumProperties = 1;
  p.StartProperty = 0x06;
  p.Data[0] = 0xff;
  sendCmd(SET_PROPERTY, &p, 6, NULL, 0);

  /*
   * AIS uses a slightly different Gaussian shape than default GMSK in the Si4463.
   * This sets the TX filter coefficients for a BT of 0.4 as per ITU specification.
   *
   * https://www.silabs.com/community/wireless/proprietary/forum.topic.html/si4463_gmsk_spectrum-ojkN
   */
  uint8_t data[] = { 0x52, 0x4f, 0x45, 0x37, 0x28, 0x1a, 0x10, 0x09, 0x04 };
  p.Group = 0x22;
  p.NumProperties = 9;
  p.StartProperty = 0x0f;
  memcpy(p.Data, data, sizeof data);
  sendCmd(SET_PROPERTY, &p, 12, NULL, 0);
}

void Transceiver::processEvent(const Event &e)
{
  switch(e.type)
  {
  case CLOCK_EVENT:
    mUTC = e.clock.utc;
    break;
  default:
    break;
  }
}

void Transceiver::noiseFloorUpdated(VHFChannel channel, uint8_t rssi)
{
  mNoiseFloorCache[channel] = rssi;
}

void Transceiver::transmitCW(VHFChannel channel)
{
  startReceiving(channel);
  configureGPIOsForTX(TX_POWER_LEVEL);
  SET_PROPERTY_PARAMS p;
  p.Group = 0x20;
  p.NumProperties = 1;
  p.StartProperty = 0x00;
  p.Data[0] = 0x08;
  sendCmd (SET_PROPERTY, &p, 4, NULL, 0);


  TX_OPTIONS options;
  options.channel = AIS_CHANNELS[channel].ordinal;
  options.condition = 8 << 4;
  options.tx_len = 0;
  options.tx_delay = 0;
  options.repeats = 0;

  sendCmd (START_TX, &options, sizeof options, NULL, 0);

#if 0
  CHIP_STATUS_REPLY chip_status;
  sendCmd (GET_CHIP_STATUS, NULL, 0, &chip_status, sizeof chip_status);
  if (chip_status.Current & 0x08)
    {
      //printf2 ("Error starting TX:\r\n");
      //printf2 ("%.8x %.8x %.8x\r\n", chip_status.Pending, chip_status.Current, chip_status.Error);
    }
  else
    {
      //printf2 ("Radio transmitting carrier on channel %d (%.3fMHz)\r\n", AIS_CHANNELS[channel].itu, AIS_CHANNELS[channel].frequency);
    }
#endif
}

void Transceiver::setTXPower(tx_power_level powerLevel)
{
  const pa_params &pwr = POWER_TABLE[powerLevel];
  SET_PROPERTY_PARAMS p;
  p.Group = 0x22;
  p.NumProperties = 3;
  p.StartProperty = 0x00;
  p.Data[0] = pwr.pa_mode;
  p.Data[1] = pwr.pa_level;
  p.Data[2] = pwr.pa_bias_clkduty;
  sendCmd(SET_PROPERTY, &p, 6, NULL, 0);
}


void Transceiver::configureGPIOsForTX(tx_power_level powerLevel)
{
  bsp_set_tx_mode();

  /*
   * Configure radio GPIOs for TX:
   * GPIO 0: Don't care
   * GPIO 1: INPUT of TX bits
   * GPIO 2: Don't care
   * GPIO 3: RX_TX_DATA_CLK
   * NIRQ  : SYNC_WORD_DETECT
   */

  GPIO_PIN_CFG_PARAMS gpiocfg;
  gpiocfg.GPIO0 = 0x20;       // TX_STATE; low during RX and high during TX
  gpiocfg.GPIO1 = 0x04;       // Input
  gpiocfg.GPIO2 = 0x1F;       // RX/TX data clock
  gpiocfg.GPIO3 = 0x21;       // RX_STATE; high in RX, low in TX
  gpiocfg.NIRQ  = 0x1A;       // Sync word detect
  gpiocfg.SDO   = 0x00;       // No change
  gpiocfg.GENCFG = 0x00;      // No change
  sendCmd(GPIO_PIN_CFG, &gpiocfg, sizeof gpiocfg, &gpiocfg, sizeof gpiocfg);

  setTXPower(powerLevel);
}

void Transceiver::startListening(VHFChannel channel)
{
  Receiver::startListening(channel);
}

void Transceiver::assignTXPacket(TXPacket *p)
{
  ASSERT(!mTXPacket);
  mTXPacket = p;
  mTXPacket->setTimestamp(mUTC);
}

TXPacket* Transceiver::assignedTXPacket()
{
  return mTXPacket;
}

void Transceiver::onBitClock()
{
  if ( gRadioState == RADIO_RECEIVING )
    {
      Receiver::onBitClock();
#ifndef TX_TEST_MODE
#ifdef ENABLE_TX
      /*
          We start transmitting a packet if:
            - We have a TX packet assigned
            - We are at bit CCA_SLOT_BIT+1 (after obtaining an RSSI level)
            - The TX packet's transmission channel is our current listening channel
            - The RSSI is within 6dB of the noise floor for this channel
            - It's been at least MIN_TX_INTERVAL seconds since our last transmission

            After PCB revision 2.0:
            - No TX packets are queued if the supercapacitor is not charged, so no need to check here.
       */

      if ( mUTC && mTXPacket && mSlotBitNumber == CCA_SLOT_BIT+1 && mTXPacket && mTXPacket->channel() == mChannel )
        {
          auto it = mNoiseFloorCache.find(mChannel);
          if ( it != mNoiseFloorCache.end() )
            {
              uint8_t noiseFloor = it->second;
              if ( mRXPacket.rssi() < noiseFloor + TX_CCA_HEADROOM )
                {
                  if ( mUTC - mTXPacket->timestamp() > MIN_MSG_18_TX_INTERVAL )
                    {
                      // The packet is way too old. Discard it.
                      TXPacketPool::instance().deleteTXPacket(mTXPacket);
                      mTXPacket = NULL;
                      //printf2("Transceiver discarded aged TX packet\r\n");
                    }
                  else if ( mUTC - mLastTXTime >= MIN_TX_INTERVAL )
                    {
                      startTransmitting();
                    }
                }
            }
        }
#endif
#else
      // In Test Mode we don't care about RSSI, SOTDMA or anything. Presumably we're firing into a dummy load ;-) Also, we don't care about throttling.
      if ( mTXPacket ) {
          startTransmitting();
      }
#endif
    }
  else
    {
      if ( mTXPacket->eof() )
        {
          mLastTXTime = mUTC;
          startReceiving(mChannel);
          gRadioState = RADIO_RECEIVING;
          reportTXEvent();
          TXPacketPool::instance().deleteTXPacket(mTXPacket);
          mTXPacket = NULL;
        }
      else
        {
          uint8_t bit = mTXPacket->nextBit();
          if ( bit )
            HAL_GPIO_WritePin(mDataPort, mDataPin, GPIO_PIN_SET);
          else
            HAL_GPIO_WritePin(mDataPort, mDataPin, GPIO_PIN_RESET);

          /**
           * As of September 2020, the digital ramp-down of the Si4463 is broken and not
           * in the latest firmware patch either, so this is a good alternative:
           *
           * We're assuming the packet has 2-3 ramp down bits, so when it tells us it's done,
           * we start ramping the PA down as the transmission ends. There is an RC delay circuit
           * on the PA bias voltage rail, so it won't "crash" hard. This really helped clean up spurs.
           *
           */
          if ( mTXPacket->canRampDown() )
            HAL_GPIO_WritePin(TX_CTRL_PORT, TX_CTRL_PIN, GPIO_PIN_RESET);
        }
    }
}


void Transceiver::timeSlotStarted(uint32_t slot)
{
  Receiver::timeSlotStarted(slot);

  // Switch channel if we have a transmission scheduled and we're not on the right channel
  if ( gRadioState == RADIO_RECEIVING && mTXPacket && mTXPacket->channel() != mChannel )
    startReceiving(mTXPacket->channel());
}

void Transceiver::startTransmitting()
{
  // Configure the RFIC GPIOs for transmission
  // Configure the pin for GPIO 1 as output
  // Set TX power level
  // Start transmitting
  gRadioState = RADIO_TRANSMITTING;
  configureGPIOsForTX(TX_POWER_LEVEL);

  //ASSERT(false);


  TX_OPTIONS options;
  options.channel     = AIS_CHANNELS[mTXPacket->channel()].ordinal;
  options.condition   = 0;
  options.tx_len      = 0;
  options.tx_delay    = 0;
  options.repeats     = 0;

  sendCmd(START_TX, &options, sizeof options, NULL, 0);


#if 0
  /*
   * Check if something went wrong
   */
  CHIP_STATUS_REPLY chip_status;
  sendCmd(GET_CHIP_STATUS, NULL, 0, &chip_status, sizeof chip_status);
  if ( chip_status.Current & 0x08 ) {
      printf2("Error starting TX: %.8x %.8x %.8x\r\n", chip_status.Pending, chip_status.Current, chip_status.Error);
      gRadioState = RADIO_RECEIVING;
      startReceiving(mChannel);
  }
#endif
}

void Transceiver::startReceiving(VHFChannel channel)
{
  Receiver::startReceiving(channel);
}

void Transceiver::configureGPIOsForRX()
{
  bsp_set_rx_mode();

  /*
   * Configure radio GPIOs for RX:
   * GPIO 0: TX_STATE
   * GPIO 1: RX_DATA
   * GPIO 2: RX_TX_DATA_CLK
   * GPIO 3: RX_STATE
   * NIRQ  : SYNC_WORD_DETECT
   */

  GPIO_PIN_CFG_PARAMS gpiocfg;
  gpiocfg.GPIO0 = 0x20;       // TX_STATE; low during RX and high during TX
  gpiocfg.GPIO1 = 0x14;       // RX data bits
  gpiocfg.GPIO2 = 0x1F;       // RX/TX data clock
  gpiocfg.GPIO3 = 0x21;       // RX_STATE; high during RX and low during TX
  gpiocfg.NIRQ  = 0x1A;       // Sync word detect
  gpiocfg.SDO   = 0x00;       // No change
  gpiocfg.GENCFG = 0x00;      // No change
  sendCmd(GPIO_PIN_CFG, &gpiocfg, sizeof gpiocfg, &gpiocfg, sizeof gpiocfg);
}

void Transceiver::reportTXEvent()
{
  Event *e = EventPool::instance().newEvent(PROPR_NMEA_SENTENCE);
  if ( !e )
    return;

  sprintf(e->nmeaBuffer.sentence, "$PAITX,%c,%s*", AIS_CHANNELS[mTXPacket->channel()].designation, mTXPacket->messageType());

  Utils::completeNMEA(e->nmeaBuffer.sentence);
  EventQueue::instance().push(e);
}

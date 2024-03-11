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
#include "TXErrors.h"
#include <stdio.h>
#include "TXScheduler.hpp"

Transceiver::Transceiver(GPIO_TypeDef *sdnPort, uint32_t sdnPin, GPIO_TypeDef *csPort,
    uint32_t csPin, GPIO_TypeDef *dataPort, uint32_t dataPin,
    GPIO_TypeDef *clockPort, uint32_t clockPin, int chipId)
: Receiver(sdnPort, sdnPin, csPort, csPin, dataPort, dataPin, clockPort, clockPin, chipId)
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

  // These resulted from characterization with default BOM
  pa_params pwr;
  switch(mPartNumber)
  {
  case 0x4463:
    pwr.pa_mode = 0x48;
    pwr.pa_level = 0x10;
    pwr.pa_bias_clkduty = 0x00;
    break;
  case 0x4467:
    pwr.pa_mode = 0x48;
    pwr.pa_level = 0x1C;
    pwr.pa_bias_clkduty = 0x00;
    break;
  default:
    pwr.pa_mode = 0x48;
    pwr.pa_level = 0x1C;
    pwr.pa_bias_clkduty = 0x00;
    break;
  }

  setTXPower(pwr);
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

void Transceiver::transmitCW(VHFChannel channel)
{
  startReceiving(channel, false);
  configureGPIOsForTX();

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
}

void Transceiver::setTXPower(const pa_params &pwr)
{
  SET_PROPERTY_PARAMS p;
  p.Group = 0x22;
  p.NumProperties = 3;
  p.StartProperty = 0x00;
  p.Data[0] = pwr.pa_mode;
  p.Data[1] = pwr.pa_level;
  p.Data[2] = pwr.pa_bias_clkduty;
  sendCmd(SET_PROPERTY, &p, 6, NULL, 0);
}


void Transceiver::configureGPIOsForTX()
{
  bsp_set_tx_mode();

  GPIO_PIN_CFG_PARAMS gpiocfg;
  gpiocfg.GPIO0 = 0x00;       // No change
  gpiocfg.GPIO1 = 0x04;       // RX/TX bit data
  gpiocfg.GPIO2 = 0x1F;       // RX/TX bit clock
#if BOARD_REV < 105
  gpiocfg.GPIO3 = 0x21;       // RX_STATE; low during TX and high during RX
#else
  gpiocfg.GPIO3 = 0x20;       // RX_STATE; high during TX and low during RX
#endif
  gpiocfg.NIRQ  = 0x00;       // No change
  gpiocfg.SDO   = 0x00;       // No change
  gpiocfg.GENCFG = 0x00;      // No change
  sendCmd(GPIO_PIN_CFG, &gpiocfg, sizeof gpiocfg, NULL, 0);
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

/**
 * This method is called in interrupt context
 */
void Transceiver::onBitClock()
{
  if ( gRadioState == RADIO_RECEIVING )
    {
      Receiver::onBitClock();
      /*
          We start transmitting a packet if:
            - We have a TX packet assigned
            - Transmission is enabled
            - We are at bit CCA_SLOT_BIT+1 (after obtaining an RSSI level)
            - The TX packet's transmission channel is our current listening channel
            - The RSSI is within 6dB of the noise floor for this channel
            - It's been at least MIN_TX_INTERVAL seconds since our last transmission
       */

      if ( !mTXPacket )
        return;


      if ( mTXPacket->isTestPacket() )
        {
          // Test packets are sent immediately. Presumably, we're firing into a dummy load ;)
          startTransmitting();
        }
      else if ( !TXScheduler::instance().isTXAllowed() )
        {
          // Transmission has been disabled since scheduling, so don't transmit!
          TXPacketPool::instance().deleteTXPacket(mTXPacket);
          mTXPacket = NULL;
        }
      else if ( mUTC && mUTC - mTXPacket->timestamp() >= MIN_MSG_18_TX_INTERVAL )
        {
          // The packet is way too old. Discard it.
#if REPORT_TX_SCHEDULING
          Event *e = EventPool::instance().newEvent(PROPR_NMEA_SENTENCE);
          if ( e )
            {
              sprintf(e->nmeaBuffer.sentence, "$PAISCHTX,%s,%d*", mTXPacket->messageType(), TX_PACKET_TOO_OLD);
              Utils::completeNMEA(e->nmeaBuffer.sentence);
              EventQueue::instance().push(e);
            }
#endif
          TXPacketPool::instance().deleteTXPacket(mTXPacket);
          mTXPacket = NULL;
        }
      else if ( mUTC - mLastTXTime < MIN_TX_INTERVAL )
        {
          // It's not time to transmit yet
          return;
        }
      else if ( mUTC && mSlotBitNumber == CCA_SLOT_BIT && mTXPacket->channel() == mChannel )
        {
          // It has already been sampled during Receiver::onBitClock();
          int rssi = mRXPacket->rssi();
          int nf = NoiseFloorDetector::instance().getNoiseFloor(AIS_CHANNELS[mChannel].designation);
          if ( rssi <= nf + TX_CCA_HEADROOM )
            {
              startTransmitting();
            }
        }
    }
  else if ( mTXPacket )
    {
      if ( mTXPacket->eof() )
        {
          mLastTXTime = mUTC;
          startReceiving(mChannel, true);
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
           * in the latest firmware patch either, so this is a good alternative.
           *
           * The packet has 2-3 ramp down bits, so when it tells us it's reached them,
           * we ramp the bias voltage down by means of the RC delay circuit.
           * This really helped clean up spurs.
           *
           */
          if ( mTXPacket->canRampDown() )
            HAL_GPIO_WritePin(PA_BIAS_PORT, PA_BIAS_PIN, GPIO_PIN_RESET);
        }
    }
  else
    {
      // We're transmitting carrier wave, do nothing. This is only done during XO trimming.
    }
}


void Transceiver::timeSlotStarted(uint32_t slot)
{
  Receiver::timeSlotStarted(slot);

  // Switch channel if we have a transmission scheduled and we're not on the right channel
  if ( gRadioState == RADIO_RECEIVING && mTXPacket && mTXPacket->channel() != mChannel )
    startReceiving(mTXPacket->channel(), false);
}

void Transceiver::startTransmitting()
{
  // Configure the RFIC GPIOs for transmission
  // Configure the pin for GPIO 1 as output
  // Set TX power level
  // Start transmitting
  gRadioState = RADIO_TRANSMITTING;
  configureGPIOsForTX();

  //ASSERT(false);


  TX_OPTIONS options;
  options.channel     = AIS_CHANNELS[mTXPacket->channel()].ordinal;
  options.condition   = 0;
  options.tx_len      = 0;
  options.tx_delay    = 0;
  options.repeats     = 0;

  sendCmd(START_TX, &options, sizeof options, NULL, 0);

  // Ensure all data changes in the function have completed, otherwise gRadioState may not actually be modified
  __DSB();

}

void Transceiver::startReceiving(VHFChannel channel, bool reconfigGPIOs)
{
  Receiver::startReceiving(channel, reconfigGPIOs);
}

void Transceiver::configureGPIOsForRX()
{
  bsp_set_rx_mode();

  GPIO_PIN_CFG_PARAMS gpiocfg;
  gpiocfg.GPIO0 = 0x00;       // No change
  gpiocfg.GPIO1 = 0x14;       // RX data bits
  gpiocfg.GPIO2 = 0x1F;       // RX/TX data clock
#if BOARD_REV < 105
  gpiocfg.GPIO3 = 0x21;       // RX_STATE; high during TX and low during RX
#else
  gpiocfg.GPIO3 = 0x20;       // RX_STATE; high during TX and low during RX
#endif
  gpiocfg.NIRQ  = 0x00;       // No change
  gpiocfg.SDO   = 0x00;       // No change
  gpiocfg.GENCFG = 0x00;      // No change
  sendCmd(GPIO_PIN_CFG, &gpiocfg, sizeof gpiocfg, NULL, 0);
}

void Transceiver::reportTXEvent()
{
  Event *e = EventPool::instance().newEvent(PROPR_NMEA_SENTENCE);
  if ( !e )
    return;

  snprintf(e->nmeaBuffer.sentence, sizeof e->nmeaBuffer.sentence, "$PAITX,%c,%s*", AIS_CHANNELS[mTXPacket->channel()].designation, mTXPacket->messageType());
  Utils::completeNMEA(e->nmeaBuffer.sentence);
  EventQueue::instance().push(e);
  bsp_tx_led_off();
}

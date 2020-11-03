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


#include "RadioManager.hpp"
#include "NoiseFloorDetector.hpp"
#include "bsp.hpp"


void rxClockCB();
void trxClockCB();


RadioManager &RadioManager::instance()
{
  static RadioManager __instance;
  return __instance;
}

RadioManager::RadioManager()
{
  mTransceiverIC = NULL;
  mReceiverIC = NULL;
  mInitializing = true;
  mTXQueue = new CircularQueue<TXPacket*>(4);
  mUTC = 0;
  EventQueue::instance().addObserver(this, CLOCK_EVENT);
}

bool RadioManager::initialized()
{
  return !mInitializing;
}

void RadioManager::init()
{
  NoiseFloorDetector::instance();
  //DBG("Initializing RF IC 1\r\n");
  mTransceiverIC = new Transceiver(SDN1_PORT, SDN1_PIN,
      CS1_PORT, CS1_PIN,
      TRX_IC_DATA_PORT, TRX_IC_DATA_PIN,
      TRX_IC_CLK_PORT, TRX_IC_CLK_PIN);
  mTransceiverIC->init();

#ifndef TX_TEST_MODE
  //DBG("Initializing RF IC 2\r\n");
  mReceiverIC = new Receiver(SDN2_PORT, SDN2_PIN,
      CS2_PORT, CS2_PIN,
      RX_IC_DATA_PORT, RX_IC_DATA_PIN,
      RX_IC_CLK_PORT, RX_IC_CLK_PIN);
  mReceiverIC->init();
#endif

  mInitializing = false;
  //DBG("Radio ICs initialized\r\n");
}

void RadioManager::transmitCW(VHFChannel channel)
{
  mTransceiverIC->transmitCW(channel);
}

void RadioManager::start()
{
  //DBG("Radio Manager starting\r\n");
  configureInterrupts();
  if ( mTransceiverIC )
    mTransceiverIC->startReceiving(CH_87);

  if ( mReceiverIC )
    mReceiverIC->startReceiving(CH_88);

  GPS::instance().setDelegate(this);
  //DBG("Radio Manager started\r\n");
}


void RadioManager::configureInterrupts()
{
  bsp_set_trx_clk_callback(trxClockCB);
  bsp_set_rx_clk_callback(rxClockCB);
}

void RadioManager::processEvent(const Event &e)
{
  mUTC = e.clock.utc;

  // Evaluate the state of the transceiver IC and our queue ...
  if ( mTransceiverIC->assignedTXPacket() == NULL )
    {
      if ( !mTXQueue->empty() )
        {
          // There is no current TX operation pending, so we assign one
          TXPacket *packet = NULL;
          mTXQueue->pop(packet);
          ASSERT(packet);

          VHFChannel txChannel = packet->channel();

          // Do we need to swap channels?
          if ( txChannel != mTransceiverIC->channel() )
            {
              //DBG("RadioManager swapping channels for ICs\r\n");
              // The receiver needs to be explicitly told to switch channels
              if ( mReceiverIC )
                mReceiverIC->switchToChannel(alternateChannel(txChannel));
            }

          //DBG("RadioManager assigned TX packet\r\n");

          // The transceiver will switch channel if the packet channel is different
          mTransceiverIC->assignTXPacket(packet);
        }

    }
}

VHFChannel RadioManager::alternateChannel(VHFChannel channel)
{
  // TODO: Delegate this to the ChannelManager
  return channel == CH_88 ? CH_87 : CH_88;
}

void RadioManager::onBitClock(uint8_t ic)
{
  if ( mInitializing )
    return;

  if ( ic == 1 && mTransceiverIC )
    mTransceiverIC->onBitClock();
  else if ( mReceiverIC )
    mReceiverIC->onBitClock();
}

void RadioManager::timeSlotStarted(uint32_t slotNumber)
{
  if ( mInitializing )
    return;

#ifndef TX_TEST_MODE
  mTransceiverIC->timeSlotStarted(slotNumber);
  mReceiverIC->timeSlotStarted(slotNumber);
#endif
}

void RadioManager::scheduleTransmission(TXPacket *packet)
{
  if ( mTXQueue->push(packet) )
    {
      //DBG("RadioManager queued TX packet for channel %d\r\n", ORDINAL_TO_ITU(packet->channel()));
    }
  else
    {
      //DBG("RadioManager rejected TX packet for channel %d\r\n", ORDINAL_TO_ITU(packet->channel()));
      TXPacketPool::instance().deleteTXPacket(packet);
    }
}

void RadioManager::sendTestPacketNow(TXPacket *packet)
{
  if ( mTransceiverIC )
    {
      mTransceiverIC->assignTXPacket(packet);
    }
}

void rxClockCB()
{
  RadioManager::instance().onBitClock(2);
}

void trxClockCB()
{
  RadioManager::instance().onBitClock(1);
}




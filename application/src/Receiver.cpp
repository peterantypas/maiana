/*
 * Receiver.cpp
 *
 *  Created on: May 21, 2016
 *      Author: peter
 */

#include "Receiver.hpp"
#include "EZRadioPRO.h"
#include "Events.hpp"
#include "EventQueue.hpp"
#include "NoiseFloorDetector.hpp"


Receiver::Receiver(SPI_TypeDef *spi, GPIO_TypeDef *sdnPort, uint16_t sdnPin, GPIO_TypeDef *csPort, uint16_t csPin, GPIO_TypeDef *gpio1Port, uint16_t gpio1Pin,
        GPIO_TypeDef *gpio3Port, uint16_t gpio3Pin)
: RFIC(spi, sdnPort, sdnPin, csPort, csPin, gpio1Port, gpio1Pin, gpio3Port, gpio3Pin)
{
    mSlotBitNumber = 0xffff;
    mSwitchAtNextSlot = false;
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
    printf2("Configuring IC\r\n");
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
    mChannel = channel;
    RX_OPTIONS options;
    options.channel = AIS_CHANNELS[channel].ordinal;
    options.condition = 0;
    options.rx_len = 0;
    options.next_state1 = 0;
    options.next_state2 = 0;
    options.next_state3 = 0;

    configureGPIOsForRX();
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

    uint8_t rssi = 0;
    uint8_t bit = GPIO_ReadInputDataBit(mGPIO1P, mGPIO1Pin);
    processNRZIBit(bit);

    if ( (mSlotBitNumber != 0xffff) && (mSlotBitNumber++ == CCA_SLOT_BIT) ) {
        rssi = readRSSI();
        rssi += mRSSIAdjustment;
        NoiseFloorDetector::instance().report(mChannel, rssi);
        mRXPacket.setRSSI(rssi);
    }
}

void Receiver::timeSlotStarted(uint32_t slot)
{
    // This should never be called while transmitting. Transmissions start after the slot boundary and end before the end of it.
    //assert(gRadioState == RADIO_RECEIVING);
    if ( gRadioState != RADIO_RECEIVING )
        printf2("    **** WTF??? Transmitting past slot boundary? **** \r\n");

    mSlotBitNumber = 0;
    if ( mBitState == BIT_STATE_IN_PACKET )
        return;

    mRXPacket.setSlot(slot);
    if ( mSwitchAtNextSlot ) {
        mSwitchAtNextSlot = false;
        startReceiving(mSwitchToChannel);
    }
}

void Receiver::processNRZIBit(uint8_t bit)
{
    if ( mLastNRZIBit == 0xff ) {
        mLastNRZIBit = bit;
        return;
    }

    uint8_t decodedBit = !(mLastNRZIBit ^ bit);

    switch (mBitState) {
        case BIT_STATE_PREAMBLE_SYNC: {
            mLastNRZIBit = bit;
            mBitWindow <<= 1;
            mBitWindow |= decodedBit;

            /*
             * By checking for the last few training bits plus the HDLC start flag,
             * we gain enough confidence that this is not random noise.
             */
            if ( mBitWindow == 0b1010101001111110 || mBitWindow == 0b0101010101111110 ) {
                mBitState = BIT_STATE_IN_PACKET;
                mRXPacket.setChannel(mChannel);
            }

            break;
        }
        case BIT_STATE_IN_PACKET: {
            if ( mRXPacket.size() >= MAX_AIS_RX_PACKET_SIZE ) {
                // Start over
                startReceiving(mChannel);
                //resetBitScanner();
                return;
            }

            if ( mOneBitCount >= 7 ) {
                // Bad packet!
                resetBitScanner();
                startReceiving(mChannel);
                return;
            }

            mLastNRZIBit = bit;
            mBitWindow <<= 1;
            mBitWindow |= decodedBit;



            if ( (mBitWindow & 0x00ff) == 0x7E ) {
                mBitState = BIT_STATE_PREAMBLE_SYNC;
                pushPacket();
                startReceiving(mChannel);
            }
            else {
                addBit(decodedBit);
            }

            break;
        }
    }

}


bool Receiver::addBit(uint8_t bit)
{
    bool result = true;
    if ( bit ) {
        ++mOneBitCount;
    }
    else {
        // Don't put stuffed bits into the packet
        if ( mOneBitCount == 5 )
            result = false;

        mOneBitCount = 0;
    }

    if ( result ) {
        mRXByte <<= 1;
        mRXByte |= bit;
        ++mBitCount;
    }

    if ( mBitCount == 8 ) {
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
    Event *p = EventPool::instance().newEvent(AIS_PACKET_EVENT);
    if ( p == NULL ) {
        printf2("AISPacket allocation failed\r\n");
        return;
    }

    p->rxPacket = mRXPacket;
    mRXPacket.reset();
    EventQueue::instance().push(p);
#else
    mRXPacket.reset();
#endif
}

void Receiver::configureGPIOsForRX()
{
    // Configure MCU pin for RFIC GPIO1 as input (RX_DATA below)
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = mGPIO1Pin;
    gpio.GPIO_Mode = GPIO_Mode_IN;
    gpio.GPIO_Speed = GPIO_Speed_Level_1;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(mGPIO1P, &gpio);


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
}



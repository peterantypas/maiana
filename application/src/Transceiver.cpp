/*
 * Transceiver.cpp
 *
 *  Created on: May 21, 2016
 *      Author: peter
 */

#include "Transceiver.hpp"
#include "NoiseFloorDetector.hpp"
#include "LEDManager.hpp"
#include "EventQueue.hpp"
#include "Events.hpp"
#include "EZRadioPRO.h"
#include "AISChannels.h"


Transceiver::Transceiver(SPI_TypeDef *spi, GPIO_TypeDef *sdnPort, uint16_t sdnPin, GPIO_TypeDef *csPort, uint16_t csPin, GPIO_TypeDef *gpio1Port, uint16_t gpio1Pin,
        GPIO_TypeDef *gpio3Port, uint16_t gpio3Pin, GPIO_TypeDef *ctxPort, uint16_t ctxPin, GPIO_TypeDef *bypPort, uint16_t bypPin)
    : Receiver(spi, sdnPort, sdnPin, csPort, csPin, gpio1Port, gpio1Pin, gpio3Port, gpio3Pin)
{
    mTXPacket = NULL;
    mCTXPort = ctxPort;
    mCTXPin = ctxPin;
    mBYPPort = bypPort;
    mBYPPin = bypPin;
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

#ifdef USE_BT_04
    /*
     * Set the TX filter coefficients for a BT of 0.4 as per ITU spec for AIS
     * http://community.silabs.com/t5/Wireless/si4463-GMSK-spectrum-spread/m-p/160063/highlight/false#M9438
     */
    uint8_t data[] = { 0x52, 0x4f, 0x45, 0x37, 0x28, 0x1a, 0x10, 0x09, 0x04 };
    p.Group = 0x22;
    p.NumProperties = 9;
    p.StartProperty = 0x0f;
    memcpy(p.Data, data, 9);
    sendCmd(SET_PROPERTY, &p, 12, NULL, 0);
#endif
}

void Transceiver::transmitCW(VHFChannel channel)
{
    startReceiving(channel);
    configureForTX(TX_POWER_LEVEL);
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
    CHIP_STATUS_REPLY chip_status;
    sendCmd (GET_CHIP_STATUS, NULL, 0, &chip_status, sizeof chip_status);
    if (chip_status.Current & 0x08) {
        printf2 ("Error starting TX:\r\n");
        printf2 ("%.8x %.8x %.8x\r\n", chip_status.Pending,
                     chip_status.Current, chip_status.Error);
    }
    else {
        printf2 ("Radio transmitting carrier on channel %d (%.3fMHz)\r\n", AIS_CHANNELS[channel].itu, AIS_CHANNELS[channel].frequency);
    }

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


void Transceiver::configureForTX(tx_power_level powerLevel)
{
    /*
     * Configure MCU pin for RFIC GPIO1 as output
     */
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = mGPIO1Pin;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_Level_1;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(mGPIO1P, &gpio);

    /*
     * Configure radio GPIOs for TX:
     * GPIO 0: Don't care
     * GPIO 1: INPUT of TX bits
     * GPIO 2: Don't care
     * GPIO 3: RX_TX_DATA_CLK
     * NIRQ  : SYNC_WORD_DETECT
     */

    GPIO_PIN_CFG_PARAMS gpiocfg;
    gpiocfg.GPIO0 = 0x00;       // No change
    gpiocfg.GPIO1 = 0x04;       // Input
    gpiocfg.GPIO2 = 0x00;       // No change
    gpiocfg.GPIO3 = 0x1F;       // RX/TX data clock
    gpiocfg.NIRQ  = 0x1A;       // Sync word detect
    gpiocfg.SDO   = 0x00;       // No change
    gpiocfg.GENCFG = 0x00;      // No change
    sendCmd(GPIO_PIN_CFG, &gpiocfg, sizeof gpiocfg, &gpiocfg, sizeof gpiocfg);

    setTXPower(powerLevel);

    const pa_params &pwr = POWER_TABLE[powerLevel];
    if ( pwr.bypass )
        GPIO_SetBits(mBYPPort, mBYPPin);
    else
        GPIO_ResetBits(mBYPPort, mBYPPin);

    // CTX goes high
    GPIO_SetBits(mCTXPort, mCTXPin);
}

void Transceiver::assignTXPacket(TXPacket *p)
{
    ASSERT(!mTXPacket);
    mTXPacket = p;
}

TXPacket* Transceiver::assignedTXPacket()
{
    return mTXPacket;
}

void Transceiver::onBitClock()
{
    if ( gRadioState == RADIO_RECEIVING ) {
        Receiver::onBitClock();

        // If we have an assigned packet and we're on the correct channel, at the right bit of the time slot,
        // and the RSSI is within 6dB of the noise floor for this channel, then fire!!!
        uint8_t noiseFloor = NoiseFloorDetector::instance().getNoiseFloor(mChannel);
        if ( mSlotBitNumber == CCA_SLOT_BIT+1 && mTXPacket && mTXPacket->channel() == mChannel && mRXPacket.rssi() < noiseFloor + 12 ) {
            startTransmitting();
        }
    }
    else {
        if ( mTXPacket->eof() ) {
            LEDManager::instance().blink(LEDManager::BLUE_LED);
            startReceiving(mChannel);
            printf2("Transmitted %d bit packet on channel %d\r\n", mTXPacket->size(), AIS_CHANNELS[mChannel].itu);
            TXPacketPool::instance().deleteTXPacket(mTXPacket);
            mTXPacket = NULL;
            gRadioState = RADIO_RECEIVING;
        }
        else {
            uint8_t bit = mTXPacket->nextBit();
            if ( bit )
                GPIO_SetBits(mGPIO1P, mGPIO1Pin);
            else
                GPIO_ResetBits(mGPIO1P, mGPIO1Pin);
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
    configureForTX(TX_POWER_LEVEL);

    //ASSERT(false);


    TX_OPTIONS options;
    options.channel     = AIS_CHANNELS[mChannel].ordinal;
    options.condition   = 0;
    options.tx_len      = 0;
    options.tx_delay    = 0;
    options.repeats     = 0;

    sendCmd(START_TX, &options, sizeof options, NULL, 0);

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
}

void Transceiver::startReceiving(VHFChannel channel)
{
    // Make sure the FEM is in RX mode - CTX goes low
    GPIO_ResetBits(mCTXPort, mCTXPin);

    Receiver::startReceiving(channel);
}



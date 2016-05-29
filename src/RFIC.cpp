/*
 * RFIC.cpp
 *
 *  Created on: May 21, 2016
 *      Author: peter
 */

#include "RFIC.hpp"
#include "radio_config.h"
#include "Utils.hpp"
#include "printf2.h"
#include "EZRadioPRO.h"
#include <cstring>
#include "_assert.h"

RFIC::RFIC( SPI_TypeDef *spi,
            GPIO_TypeDef *sdnPort,
            uint16_t sdnPin,
            GPIO_TypeDef *csPort,
            uint16_t csPin,
            GPIO_TypeDef *gpio1Port,
            uint16_t gpio1Pin,
            GPIO_TypeDef *gpio3Port,
            uint16_t gpio3Pin)
{
    mSPI = spi;
    mSDNP = sdnPort;
    mCSP = csPort;
    mGPIO1P = gpio1Port;
    mGPIO3P = gpio3Port;

    mSDNPin = sdnPin;
    mCSPin = csPin;
    mGPIO1Pin = gpio1Pin;
    mGPIO3Pin = gpio3Pin;

    mRSSIAdjustment = 0;
    mSPIBusy = false;

    if ( !isUp() )
        powerOnReset();

}

RFIC::~RFIC()
{
}

void RFIC::setRSSIAdjustment(short adj)
{
    mRSSIAdjustment = adj;
}

void RFIC::spiOn()
{
    GPIO_ResetBits(mCSP, mCSPin);
}

void RFIC::spiOff()
{
    GPIO_SetBits(mCSP, mCSPin);
}

uint8_t RFIC::sendSPIByte(uint8_t data)
{
    /*
     * See this URL for an explanation:
    http://www.eevblog.com/forum/microcontrollers/stm32f0-spi-8bit-communication-frame-(errata)/
    */

    while((mSPI->SR & SPI_I2S_FLAG_TXE)==RESET)
        ;

    SPI_SendData8(mSPI, data);

    while((mSPI->SR & SPI_I2S_FLAG_RXNE)==RESET)
        ;

    return SPI_ReceiveData8(mSPI);
}

void RFIC::sendCmd(uint8_t cmd, void* params, uint8_t paramLen, void* result, uint8_t resultLen)
{
    ASSERT(!mSPIBusy);

    mSPIBusy = true;
    spiOn();
    sendSPIByte(cmd);

    uint8_t *b = (uint8_t*) params;
    for ( int i = 0; i < paramLen; ++i ) {
        sendSPIByte(b[i]);
    }

    spiOff(); // The Si446x needs CS asserted after every request to trigger an internal interrupt

    while (receiveSPIData(result, resultLen) == false)
        ;   // wait for valid response

    mSPIBusy = false;
}

// This is borrowed from the dAISy project. Thank you Adrian :)
bool RFIC::receiveSPIData(void *data, uint8_t length)
{
    spiOn();
    sendSPIByte(0x44);
    if ( sendSPIByte(0) != 0xff ) {
        spiOff();
        return false;
    }

    uint8_t* b = (uint8_t*) data;
    uint8_t i = 0;
    while (i < length) {
        b[i] = sendSPIByte(0);
        ++i;
    }

    spiOff();
    return true;
}

void RFIC::configure()
{
    uint8_t radio_configuration[] = RADIO_CONFIGURATION_DATA_ARRAY;
    uint8_t *cfg = radio_configuration;
    while (*cfg) {                              // configuration array stops with 0
        uint8_t count = (*cfg++) - 1;           // 1st byte: number of bytes, incl. command
        uint8_t cmd = *cfg++;                   // 2nd byte: command
        sendCmd(cmd, cfg, count, NULL, 0);      // send bytes to chip
        cfg += count;                           // point at next line
    }
}

bool RFIC::isUp()
{
    GPIO_SetBits(mSDNP, mSDNPin);
    Utils::delay(1000000);
    GPIO_ResetBits(mSDNP, mSDNPin);
    Utils::delay(10000000);

    printf2("Checking RF chip status\r\n");
    CHIP_STATUS_REPLY chip_status;
    memset(&chip_status, 0, sizeof chip_status);
    sendCmd(GET_CHIP_STATUS, NULL, 0, &chip_status, sizeof chip_status);
    printf2("Chip status: %.2x\r\n", chip_status.Current);
    if ( chip_status.Current & 0x08 ) {
        return false;
    }
    else {
        return true;
    }
}

void RFIC::powerOnReset()
{
    // Put SDN high
    GPIO_SetBits(mSDNP, mSDNPin);

    // Wait at least 10 microseconds
    Utils::delay(500000);

    // Put SDN low and poll the status of GPIO1
    GPIO_ResetBits(mSDNP, mSDNPin);
    printf2("Waiting for GPIO1\r\n");
    while ( GPIO_ReadInputDataBit(mGPIO1P, mGPIO1Pin) == 0 )
        ;

    // We're done!
    printf2("Radio Ready!\r\n");
}

uint8_t RFIC::readRSSI()
{
    MODEM_STATUS_REPLY s;
    sendCmd(GET_MODEM_STATUS, NULL, 0, &s, sizeof s);
    return s.CurrentRSSI;
}

bool RFIC::checkStatus()
{
    DEVICE_STATE s;
    sendCmd(REQ_DEVICE_STATE, NULL, 0, &s, sizeof s);
    if ( s.state != 8 && s.state != 7 )
        return false;
    else
        return true;
}






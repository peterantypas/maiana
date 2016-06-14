/*
 * RFIC.hpp
 *
 *  Created on: May 21, 2016
 *      Author: peter
 */

#ifndef RFIC_HPP_
#define RFIC_HPP_

#include <inttypes.h>
#include "stm32f30x.h"


typedef enum
{
    BIT_STATE_PREAMBLE_SYNC,
    BIT_STATE_IN_PACKET
} BitState;

class RFIC
{
public:
    RFIC(   SPI_TypeDef *spi,
            GPIO_TypeDef *sdnPort,
            uint16_t sdnPin,
            GPIO_TypeDef *csPort,
            uint16_t csPin,
            GPIO_TypeDef *gpio1Port,
            uint16_t gpio1Pin,
            GPIO_TypeDef *gpio3Port,
            uint16_t gpio3Pin);
    virtual ~RFIC();

    void setRSSIAdjustment(short rssiAdj);
protected:
    virtual void configure();
    void sendCmd(uint8_t cmd, void* params, uint8_t paramLen, void* result, uint8_t resultLen);
    bool isUp();
    void powerOnReset();
    bool isReceiving();
    uint8_t readRSSI();
    bool checkStatus();
private:
    uint8_t sendSPIByte(uint8_t);
    bool receiveSPIData(void *data, uint8_t len);
    void spiOn();
    void spiOff();
protected:
    SPI_TypeDef     *mSPI;              // The SPI bus we use
    GPIO_TypeDef    *mSDNP;             // The MCU GPIO assigned to SDN for this IC (GPIOA, GPIOB or GPIOC)
    GPIO_TypeDef    *mCSP;              // The MCU GPIO assigned to CS for SPI
    GPIO_TypeDef    *mGPIO1P;           // The MCU GPIO assigned to GPIO1 of the IC
    GPIO_TypeDef    *mGPIO3P;           // The MCU GPIO assigned to GPIO3 of the IC
    uint16_t        mSDNPin;
    uint16_t        mCSPin;
    uint16_t        mGPIO1Pin;
    uint16_t        mGPIO3Pin;

    uint8_t         mLastNRZIBit;
    BitState        mBitState;
    short           mRSSIAdjustment;
    bool            mSPIBusy;
};

#endif /* RFIC_HPP_ */

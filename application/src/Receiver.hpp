/*
 * Receiver.hpp
 *
 *  Created on: May 21, 2016
 *      Author: peter
 */

#ifndef RECEIVER_HPP_
#define RECEIVER_HPP_

#include "RXPacket.hpp"
#include "RadioState.hpp"
#include "RFIC.hpp"
#include "AISChannels.h"



class Receiver : public RFIC
{
public:
    Receiver(SPI_TypeDef *spi,
            GPIO_TypeDef *sdnPort,
            uint16_t sdnPin,
            GPIO_TypeDef *csPort,
            uint16_t csPin,
            GPIO_TypeDef *gpio1Port,
            uint16_t gpio1Pin,
            GPIO_TypeDef *gpio3Port,
            uint16_t gpio3Pin);
    virtual ~Receiver();

    bool init();
    VHFChannel channel();
    virtual void startReceiving(VHFChannel channel);
    virtual void onBitClock();
    virtual void timeSlotStarted(uint32_t slot);
    void processNRZIBit(uint8_t level);
    bool addBit(uint8_t bit);
    void pushPacket();
    void startListening(VHFChannel channel);
    void switchToChannel(VHFChannel channel);
private:
    void resetBitScanner();
protected:
    virtual void configureGPIOsForRX();
protected:
    RXPacket mRXPacket;
    uint16_t mBitWindow;
    uint8_t mLastNRZIBit;
    uint32_t mBitCount;
    uint32_t mOneBitCount;
    BitState mBitState;
    uint8_t mRXByte;
    VHFChannel mChannel;
    uint16_t mSlotBitNumber;
    bool mSwitchAtNextSlot;
    VHFChannel mSwitchToChannel;
};

#endif /* RECEIVER_HPP_ */

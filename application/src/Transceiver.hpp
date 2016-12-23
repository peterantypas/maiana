/*
 * Transceiver.hpp
 *
 *  Created on: May 21, 2016
 *      Author: peter
 */

#ifndef TRANSCEIVER_HPP_
#define TRANSCEIVER_HPP_

#include "Receiver.hpp"
#include "TXPacket.hpp"
#include "EventQueue.hpp"

class Transceiver : public Receiver, EventConsumer
{
public:
    Transceiver(SPI_TypeDef *spi,
                GPIO_TypeDef *sdnPort,
                uint16_t sdnPin,
                GPIO_TypeDef *csPort,
                uint16_t csPin,
                GPIO_TypeDef *gpio1Port,
                uint16_t gpio1Pin,
                GPIO_TypeDef *gpio3Port,
                uint16_t gpio3Pin,
                GPIO_TypeDef *ctxPort,
                uint16_t ctxPin);


    void onBitClock();
    void timeSlotStarted(uint32_t slot);
    void assignTXPacket(TXPacket *p);
    TXPacket *assignedTXPacket();
    void startReceiving(VHFChannel channel);
    void transmitCW(VHFChannel channel);
    void processEvent(const Event &);
protected:
    void configure();
    void configureGPIOsForRX();
private:
    void startTransmitting();
    void configureGPIOsForTX(tx_power_level pwr);
    void setTXPower(tx_power_level pwr);
private:
    TXPacket    *mTXPacket;
    GPIO_TypeDef *mCTXPort;
    uint16_t    mCTXPin;
    time_t      mUTC;
    time_t      mLastTXTime;
};

#endif /* TRANSCEIVER_HPP_ */

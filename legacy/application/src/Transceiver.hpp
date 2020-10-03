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


class Transceiver : public Receiver
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
                uint16_t ctxPin,
                GPIO_TypeDef *bypPort,
                uint16_t bypPin);

    void onBitClock();
    void timeSlotStarted(uint32_t slot);
    void assignTXPacket(TXPacket *p);
    TXPacket *assignedTXPacket();
    void startReceiving(VHFChannel channel);
    void transmitCW(VHFChannel channel);
protected:
    void configure();
private:
    void startTransmitting();
    void configureForTX(tx_power_level pwr);
    void setTXPower(tx_power_level pwr);
private:
    TXPacket    *mTXPacket;
    GPIO_TypeDef *mCTXPort;
    uint16_t    mCTXPin;
    GPIO_TypeDef *mBYPPort;
    uint16_t    mBYPPin;
};

#endif /* TRANSCEIVER_HPP_ */

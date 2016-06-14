/*
 * RadioManager.hpp
 *
 *  Created on: May 21, 2016
 *      Author: peter
 */

#ifndef RADIOMANAGER_HPP_
#define RADIOMANAGER_HPP_

#include "Receiver.hpp"
#include "Transceiver.hpp"
#include "GPS.hpp"
#include "TXPacket.hpp"
#include "CircularQueue.hpp"
#include "EventQueue.hpp"
#include "AISChannels.h"


class RadioManager : public GPSDelegate, public EventConsumer
{
public:
    static RadioManager &instance();

    void init();
    void start();
    void onBitClock(uint8_t ic);
    void timeSlotStarted(uint32_t slotNumber);

    void scheduleTransmission(TXPacket *p);
    bool initialized();

    void processEvent(Event *e);
    void transmitCW(VHFChannel channel);
    VHFChannel alternateChannel(VHFChannel channel);
private:
    RadioManager();
    void spiOff();
    void configureInterrupts();
private:
    Transceiver *mTransceiverIC;
    Receiver *mReceiverIC;
    bool mInitializing;

    CircularQueue<TXPacket*>    *mTXQueue;
    time_t mUTC;
};

#endif /* RADIOMANAGER_HPP_ */

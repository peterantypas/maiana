/*
 * RadioManager.cpp
 *
 *  Created on: May 21, 2016
 *      Author: peter
 */

#include "RadioManager.hpp"

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
    mTXQueue = new CircularQueue<TXPacket*>(5);
    mUTC = 0;
    EventQueue::instance().addObserver(this, CLOCK_EVENT);
}

bool RadioManager::initialized()
{
    return !mInitializing;
}

void RadioManager::init()
{
    printf2("Starting Radio layer initialization\r\n");
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);


    // Pins PA5 PA6 and PA7 will be SCLK, MISO and MOSI
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_Level_2;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

    GPIO_Init(GPIOA, &gpio);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_5);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_5);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_5);

    // PB1 will be the CS2
    gpio.GPIO_Pin = GPIO_Pin_1;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_Level_2;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &gpio);

    // PB8 is SDN2
    gpio.GPIO_Pin = GPIO_Pin_8;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_Level_1;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &gpio);

    // PC13 starts as input, used for POR status (RFIC2 GPIO1 -> MCU), then switched between RX/TX DATA as necessary
    gpio.GPIO_Pin = GPIO_Pin_13;
    gpio.GPIO_Mode = GPIO_Mode_IN;
    gpio.GPIO_Speed = GPIO_Speed_Level_1;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &gpio);

    // PB9 is input for bit clock, both RX and TX (RFIC2 GPIO3 -> MCU)
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_IN;
    gpio.GPIO_Speed = GPIO_Speed_Level_1;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &gpio);


    // PA4 will be the CS1 pin
    gpio.GPIO_Pin = GPIO_Pin_4;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_Level_2;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &gpio);

    // PA3 will be the SDN1 pin
    gpio.GPIO_Pin = GPIO_Pin_3;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_Level_1;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &gpio);

    // PB2 starts as input, used for POR status (RFIC1 GPIO1 -> MCU), then switched between RX/TX DATA as necessary
    gpio.GPIO_Pin = GPIO_Pin_2;
    gpio.GPIO_Mode = GPIO_Mode_IN;
    gpio.GPIO_Speed = GPIO_Speed_Level_1;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &gpio);

    // PA1 is input for bit clock, both RX and TX (RFIC1 GPIO3 -> MCU)
    gpio.GPIO_Pin = GPIO_Pin_1;
    gpio.GPIO_Mode = GPIO_Mode_IN;
    gpio.GPIO_Speed = GPIO_Speed_Level_1;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &gpio);



    // PA8 will be CTX
    gpio.GPIO_Pin = GPIO_Pin_8;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_Level_1;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &gpio);

    // PA9 will be BYP
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_Level_1;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &gpio);


    spiOff();

    // Initialize the SPI bus
    SPI_InitTypeDef spi;
    spi.SPI_BaudRatePrescaler   = SPI_BaudRatePrescaler_8;
    spi.SPI_Direction           = SPI_Direction_2Lines_FullDuplex;
    spi.SPI_Mode                = SPI_Mode_Master;
    spi.SPI_DataSize            = SPI_DataSize_8b;
    spi.SPI_NSS                 = SPI_NSS_Soft;
    spi.SPI_FirstBit            = SPI_FirstBit_MSB;
    spi.SPI_CPOL                = SPI_CPOL_Low;
    spi.SPI_CPHA                = SPI_CPHA_1Edge;
    spi.SPI_CRCPolynomial       = 0x01;

    /*
     * See this URL for an explanation:
    http://www.eevblog.com/forum/microcontrollers/stm32f0-spi-8bit-communication-frame-(errata)/
    */

    SPI_Init(SPI1, &spi);
    SPI_CalculateCRC(SPI1, DISABLE);
    SPI_Cmd(SPI1, ENABLE);
    SPI_RxFIFOThresholdConfig(SPI1, SPI_RxFIFOThreshold_QF);

    // This is IC1 (transceiver)
    printf2("Initializing RF IC 1\r\n");
    mTransceiverIC = new Transceiver(SPI1, GPIOA, GPIO_Pin_3, GPIOA, GPIO_Pin_4, GPIOB, GPIO_Pin_2, GPIOA, GPIO_Pin_1, GPIOA, GPIO_Pin_8, GPIOA, GPIO_Pin_9);
    mTransceiverIC->init();

    // This is IC2 (receiver)
    printf2("Initializing RF IC 2\r\n");
    mReceiverIC = new Receiver(SPI1, GPIOB, GPIO_Pin_8, GPIOB, GPIO_Pin_1, GPIOC, GPIO_Pin_13, GPIOB, GPIO_Pin_9);
    mReceiverIC->init();
    mReceiverIC->setRSSIAdjustment(-12); // IC2 gets 6dB more signal than IC1 because of PCB layout
    mInitializing = false;
}

void RadioManager::spiOff()
{
    GPIO_SetBits(GPIOB, GPIO_Pin_1);
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
}

void RadioManager::transmitCW(VHFChannel channel)
{
    mTransceiverIC->transmitCW(channel);
}

void RadioManager::start()
{
    configureInterrupts();
    mTransceiverIC->startReceiving(CH_87);
    mReceiverIC->startReceiving(CH_88);
    GPS::instance().setDelegate(this);
}


void RadioManager::configureInterrupts()
{
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;


    // Set up interrupt for PB9 (RX/TX bit clock) on EXTI line 9
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource9);
    EXTI_InitStruct.EXTI_Line = EXTI_Line9;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // Set up interrupt for PA1 (RX/TX bit clock) on EXTI line 1
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource1);
    EXTI_InitStruct.EXTI_Line = EXTI_Line1;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

}

void RadioManager::processEvent(const Event &e)
{
    //ClockEvent *ce = static_cast<ClockEvent*>(e);
    mUTC = e.clock.utc;

    // Evaluate the state of the transceiver IC and our queue ...
    if ( mTransceiverIC->assignedTXPacket() == NULL ) {
        if ( !mTXQueue->empty() ) {
            // There is no current TX operation pending, so we assign one
            TXPacket *packet = NULL;
            mTXQueue->pop(packet);
            ASSERT(packet);

            VHFChannel txChannel = packet->channel();

            // Do we need to swap channels?
            if ( txChannel != mTransceiverIC->channel() ) {
                //printf2("RadioManager swapping channels for ICs\r\n");
                // The receiver needs to be explicitly told to switch channels
                mReceiverIC->switchToChannel(alternateChannel(txChannel));
            }

            //printf2("RadioManager assigned TX packet\r\n");

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

    if ( ic == 1 )
        mTransceiverIC->onBitClock();
    else
        mReceiverIC->onBitClock();
}

void RadioManager::timeSlotStarted(uint32_t slotNumber)
{
    if ( mInitializing )
        return;

    mTransceiverIC->timeSlotStarted(slotNumber);
    mReceiverIC->timeSlotStarted(slotNumber);
}

void RadioManager::scheduleTransmission(TXPacket *packet)
{
    if ( mTXQueue->push(packet) ) {
        //printf2("RadioManager queued TX packet for channel %d\r\n", packet->channel());
    }
    else {
        printf2("RadioManager rejected TX packet for channel %d\r\n", packet->channel());
        TXPacketPool::instance().deleteTXPacket(packet);
    }
}

extern "C" {

void EXTI9_5_IRQHandler(void)
{
    if ( EXTI_GetITStatus(EXTI_Line9) != RESET ) {
        RadioManager::instance().onBitClock(2);
        EXTI_ClearITPendingBit(EXTI_Line9);
    }
}

void EXTI1_IRQHandler(void)
{
    if ( EXTI_GetITStatus(EXTI_Line1) != RESET ) {
        RadioManager::instance().onBitClock(1);
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

}


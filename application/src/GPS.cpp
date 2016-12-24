/*
 * GPS.cpp
 *
 *  Created on: Nov 9, 2015
 *      Author: peter
 */

#include "GPS.hpp"
#include "stm32f30x.h"
#include <cstdio>
#include <cassert>
#include <cstring>

#include "DataTerminal.hpp"
#include "NMEASentence.hpp"
#include "Utils.hpp"
#include "EventQueue.hpp"
#include "printf2.h"
#include "globals.h"
//#include "LEDManager.hpp"


GPS &
GPS::instance()
{
    static GPS __instance;
    return __instance;
}

GPS::GPS()
        : mBuffPos(0), mUTC(0), mLat(0), mLng(0), mStarted(false), mSlotNumber(0), mDelegate(NULL), mCOG(511), mSpeed(0)
{
    mPeriod = (SystemCoreClock / 37.5) - 1;
    memset(&mTime, 0, sizeof(mTime));
}

GPS::~GPS()
{

}

time_t GPS::UTC()
{
    return mUTC;
}

struct tm &GPS::time()
{
    return mTime;
}

uint32_t GPS::aisSlot()
{
    return mSlotNumber;
}

double GPS::lat()
{
    return mLat;
}

double GPS::lng()
{
    return mLng;
}

void GPS::setDelegate(GPSDelegate *delegate)
{
    mDelegate = delegate;
}

void GPS::init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE); // For RX
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_7);

    // Initialize pins as alternative function
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_1;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA0 is connected to EXTI_Line0  -- currently the PPS signal from the GPS */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_1;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
    EXTI_InitTypeDef EXTI_InitStruct;
    EXTI_InitStruct.EXTI_Line = EXTI_Line0;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_Init(&EXTI_InitStruct);



    /**
     * Enable clock for USART1 peripheral
     */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    USART_InitStruct.USART_BaudRate = 9600;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStruct);
    USART_Cmd(USART1, ENABLE);

    EventQueue::instance().addObserver(this, GPS_NMEA_SENTENCE);


    /**
     * Enable RX interrupt
     */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_Init(&NVIC_InitStruct);


    /*
     * Enable clock for TIM2 on APB1
     */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    resetTimer();

    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    printf2("GPS Initialized\r\n");
}

void GPS::onRX(char c)
{
    mBuff[mBuffPos++] = c;
    if (mBuffPos > 90) {
        // We screwed up!
        mBuffPos = 0;
        mBuff[mBuffPos] = 0;
    }
    else if (c == '\n') {
        mBuff[mBuffPos] = 0;
        Event *event = EventPool::instance().newEvent(GPS_NMEA_SENTENCE);
        if ( event ) {
            memcpy(event->nmeaBuffer.sentence, mBuff, sizeof event->nmeaBuffer.sentence);
            EventQueue::instance ().push (event);
        }
        mBuffPos = 0;
        mBuff[mBuffPos] = 0;
    }
}

void GPS::onPPS()
{
    if (mUTC) {
        ++mUTC;     // PPS := advance clock by one second!
        localtime_r (&mUTC, &mTime); // Now we know exactly what UTC second it is, with only microseconds of latency
        if (!mStarted) {
            // To keep things simple, we only start the AIS slot timer if we're on an even second (it has a 37.5 Hz frequency)
            mSlotNumber = (mTime.tm_sec % 60) * 2250; // We know what AIS slot number we're in
            if (!(mTime.tm_sec & 0x00000001))
                startTimer ();
        }
        else {
            // The timer is on, now let's re-calibrate ...
            if (mTime.tm_sec & 0x00000001) {
                // On odd seconds, we expect the timer value to be half its period. Just correct it.
                uint32_t nominalTimerValue = mPeriod / 2 - 1;
                TIM2->CNT = nominalTimerValue;
            }
            else {
                // On even seconds, things are a bit more tricky ...
                uint32_t currentTimerValue = TIM2->CNT;
                if (currentTimerValue >= mPeriod / 2 - 1) {
                    // The timer is a little behind, so kick it forward
                    TIM2->CNT = mPeriod - 1;
                }
                else {
                    // The timer is a little ahead, so pull it back
                    TIM2->CNT = 0;
                }
            }
        }

        Event *event = EventPool::instance().newEvent(CLOCK_EVENT);
        if ( event ) {
            event->clock.utc = mUTC;
            EventQueue::instance ().push(event);
        }
    } // We have a valid UTC timestamp

}

void GPS::processEvent(const Event &event)
{
    //printf2("-> GPS::processEvent()\r\n");

    processLine(event.nmeaBuffer.sentence);
    //printf2("<- GPS::processEvent()\r\n");
}

void GPS::processLine(const char* buff)
{
    if ( buff[0] == '$' && buff[1] != '$' ) {
        unsigned reportedHash;
        char *starPos = strstr(buff, "*");
        if ( starPos && sscanf(starPos + 1, "%x", &reportedHash) == 1 ) {
            unsigned actualHash = 0;
            for ( const char* c = buff + 1; c < starPos; ++c )
                actualHash ^= *c;

            if ( reportedHash == actualHash ) {
                parseSentence(buff);
            }

        }
    }
}

void
GPS::parseSentence(const char *buff)
{
    //printf2(buff);
    //if ( strstr(buff, "RMC") != buff+3 )
    //  return;

    NMEASentence sentence (buff);
    if (sentence.code ().find ("RMC") == 2) {
#ifdef OUTPUT_GPS_NMEA
#ifdef MULTIPLEXED_OUTPUT
        DataTerminal::instance ().write ("NMEA", buff);
#else
        DataTerminal::instance().write(buff);
#endif
#endif

        const vector<string> &fields = sentence.fields ();

        /*
         * Sometimes the GPS indicates errors with sentences like
         * $GPRMC,1420$0*74\r\n
         * Although the sentence structure is valid, its content is not what we expect.
         *
         * TODO: Should we consider the GPS non-functioning at this point and thus prevent transmission until it recovers?
         */
        if (fields.size () < 10)
            return;

        // GPS updates arrive even with no time or fix information
        if (fields[1].length () < 6 || fields[9].length () < 6)
            return;


        // This is the time that corresponds to the previous PPS
        const string &timestr = fields[1].substr (0, 6);
        const string &datestr = fields[9].substr (0, 6);
        mTime.tm_hour = Utils::toInt (timestr.substr (0, 2));
        mTime.tm_min = Utils::toInt (timestr.substr (2, 2));
        mTime.tm_sec = Utils::toInt (timestr.substr (4, 2));
        mTime.tm_mday = Utils::toInt (datestr.substr (0, 2));
        mTime.tm_mon = Utils::toInt (datestr.substr (2, 2)) - 1; // Month is 0-based
        mTime.tm_year = 100 + Utils::toInt (datestr.substr (4, 2)); // Year is 1900-based
        mUTC = mktime (&mTime);

        // Do we have a fix?
        if (mUTC && sentence.fields()[3].length() > 0 && sentence.fields()[5].length() > 0) {
            mLat = Utils::latitudeFromNMEA (sentence.fields()[3], sentence.fields()[4]);
            mLng = Utils::longitudeFromNMEA (sentence.fields()[5], sentence.fields()[6]);
            mSpeed = Utils::toDouble (sentence.fields()[7]);
            mCOG = Utils::toDouble (sentence.fields()[8]);
            Event *event = EventPool::instance ().newEvent(GPS_FIX_EVENT);
            if (event) {
                event->gpsFix.utc = mUTC;
                event->gpsFix.lat = mLat;
                event->gpsFix.lng = mLng;
                event->gpsFix.speed = mSpeed;
                event->gpsFix.cog = mCOG;
                EventQueue::instance().push (event);
            }
            //printf2("Lat: %f, Lng: %f\r\n", mLat, mLng);
        }
    }
}

void GPS::resetTimer()
{
    TIM_TimeBaseInitTypeDef timerInitStructure;
    timerInitStructure.TIM_Prescaler = 0;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = mPeriod;
    timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIM2, &timerInitStructure);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

void GPS::startTimer()
{
    TIM_Cmd(TIM2, ENABLE);
    mStarted = true;
    printf2("Started SOTDMA timer\r\n");
}

void GPS::stopTimer()
{
    mStarted = false;
    TIM_Cmd(TIM2, DISABLE);
    printf2("Stopped SOTDMA timer\r\n");
}


void GPS::reset()
{
    stopTimer();
    resetTimer();
    startTimer();
}

void GPS::onTimerIRQ()
{
    if ( mStarted ) {
        ++mSlotNumber;
        if ( mSlotNumber == 2250 )
            mSlotNumber = 0;

        // Delegates need real-time information
        if ( mDelegate )
            mDelegate->timeSlotStarted(mSlotNumber);
      }
}

extern "C" {


void TIM2_IRQHandler(void)
{
    if ( TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET ) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        GPS::instance().onTimerIRQ();
    }
}

void EXTI0_IRQHandler(void)
{
    if ( EXTI_GetITStatus(EXTI_Line0) != RESET ) {
        EXTI_ClearITPendingBit(EXTI_Line0);
        GPS::instance().onPPS();
    }
}

void USART1_IRQHandler(void)
{
    if ( USART_GetITStatus(USART1, USART_IT_RXNE) ) {
        char c = (char) USART1->RDR; // This clears the interrupt right away
        GPS::instance().onRX(c);
    }
}


}


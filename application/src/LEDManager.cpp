/*
 * LEDManager.cpp
 *
 *  Created on: Dec 25, 2015
 *      Author: peter
 */

#include "LEDManager.hpp"
#include "stm32f30x.h"
#include <cstring>


LEDManager &LEDManager::instance()
{
    static LEDManager __instance;
    return __instance;
}


LEDManager::LEDManager()
    : mLEDState({ {GPIOB, GPIO_Pin_12, 0}, {GPIOA, GPIO_Pin_9, 0} })
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE); // For LED1
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE); // For LED2
    

    for ( int i = 0; i < 2; ++i ) {
        GPIO_InitStruct.GPIO_Pin = mLEDState[i].pin;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_1;
        GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(mLEDState[i].gpio, &GPIO_InitStruct);
    }

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // The desired frequency is 20Hz so we can "blink" LEDs by toggling them at every tick (10Hz)
    uint32_t timerPeriod = (SystemCoreClock / 20) - 1;
    timerPeriod /= 10000;

    TIM_TimeBaseInitTypeDef timerInitStructure;
    timerInitStructure.TIM_Prescaler = 10000;
    timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    timerInitStructure.TIM_Period = timerPeriod;
    timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 10;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_TimeBaseInit(TIM3, &timerInitStructure);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE);

    clear();
}

LEDManager::~LEDManager()
{
}

void LEDManager::clear()
{
    for ( int i = 0; i < 2; ++i ) {
        GPIO_ResetBits(mLEDState[i].gpio, mLEDState[i].pin);
        mLEDState[i].state = 0;
    }
}

void LEDManager::onTimer()
{
    for ( int i = 0; i < 2; ++i ) {
        if ( mLEDState[i].state ) {
            GPIO_ResetBits(mLEDState[i].gpio, mLEDState[i].pin);
            mLEDState[i].state = 0;
        }
    }
}

void LEDManager::blink(uint8_t led)
{
    GPIO_SetBits(mLEDState[led].gpio, mLEDState[led].pin);
    mLEDState[led].state = 1;
}

extern "C" {

    void TIM3_IRQHandler(void)
    {
        if ( TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET ) {
            TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
            LEDManager::instance().onTimer();
        }
    }


}




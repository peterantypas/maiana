/*
 * printf2.c
 *
 *  Created on: Mar 1, 2016
 *      Author: peter
 */

#include "stm32f30x.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "Events.hpp"
#include "Utils.hpp"
#include "globals.h"
#include "EventQueue.hpp"
#include <diag/Trace.h>
#include "DataTerminal.hpp"


void printf2_Init(int baudrate)
{
#ifndef MULTIPLEXED_OUTPUT
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_7);

    USART_InitTypeDef USART_InitStructure;
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_Init(USART2, &USART_InitStructure);

    USART_Cmd(USART2, ENABLE);
#endif
}


#ifndef MULTIPLEXED_OUTPUT
void USART_putc(USART_TypeDef* USARTx, char c)
{
    while (!(USARTx->ISR & 0x00000040))
        ;
    USART_SendData(USARTx, c);
}


void USART_puts(USART_TypeDef* USARTx, const char *s)
{
    for ( int i = 0; s[i] != 0; ++i )
        USART_putc(USARTx, s[i]);
}
#endif

static char __buffer[128];

void printf2_now(const char *format, ...)
{
    va_list list;
    va_start(list, format);
    vsnprintf(__buffer, 128, format, list);
    va_end(list);
#ifdef MULTIPLEXED_OUTPUT
    DataTerminal::instance().write("DEBUG", __buffer);
#else
    USART_puts(USART2, __buffer);
#endif
}

#ifdef ENABLE_PRINTF2
void printf2(const char *format, ...)
{
    if ( Utils::inISR() ) {
        Event *e = EventPool::instance().newEvent(DEBUG_EVENT);
        if ( e == NULL )
            return;

        va_list list;
        va_start(list, format);
        vsnprintf(e->debugMessage.buffer, 128, format, list);
        va_end(list);

        EventQueue::instance().push(e);
    }
    else {
        va_list list;
        va_start(list, format);
        vsnprintf(__buffer, 128, format, list);
        va_end(list);
#ifdef MULTIPLEXED_OUTPUT
        DataTerminal::instance().write("DEBUG", __buffer);
#else
        USART_puts(USART2, __buffer);
#endif
    }
}
#else
void printf2(const char *format, ...)
{
    va_list list;
    va_start(list, format);
    vsnprintf(__buffer, 255, format, list);
    va_end(list);
    trace_printf(__buffer);
}
#endif



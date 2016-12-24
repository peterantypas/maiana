/*
 * Terminal.cpp
 *
 *  Created on: Apr 19, 2016
 *      Author: peter
 */

#include "DataTerminal.hpp"

#include "stm32f30x.h"
#include "printf2.h"
#include "Events.hpp"
#include <stdio.h>
#include "Utils.hpp"
#include <cstring>


static char __buff[128];

DataTerminal &DataTerminal::instance()
{
    static DataTerminal __instance;
    return __instance;
}

void DataTerminal::init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE); // For USART2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_7);

    // Initialize pins as alternative function 7 (USART)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_1;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);


    USART_InitTypeDef USART_InitStructure;
    USART_StructInit(&USART_InitStructure);

    USART_InitStructure.USART_BaudRate = 38400;
    USART_Init(USART2, &USART_InitStructure);
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;

    USART_Cmd(USART2, ENABLE);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_Init(&NVIC_InitStruct);
}

DataTerminal::DataTerminal()
    : mCmdBuffPos(0)
{
    mCmdTokens.reserve(5);
    EventQueue::instance().addObserver(this, RESPONSE_EVENT);
}

void DataTerminal::processEvent(const Event &e)
{
    switch (e.type) {
        case RESPONSE_EVENT:
            sprintf(__buff, "[%s]%s\r\n", (e.response.success ? "OK" : "ERR"), e.response.data);
            write("RESPONSE", __buff);
            break;
        default:
            break;
    }
}

void DataTerminal::processCharacter(char c)
{
    if ( c == '\n' ) {
        mCmdBuffer[mCmdBuffPos] = 0;
        mCmdBuffPos = 0;
        processCommand();
    }
    else {
        mCmdBuffer[mCmdBuffPos++] = c;
        if (mCmdBuffPos >= sizeof mCmdBuffer)
            mCmdBuffPos = 0;

    }
}

void DataTerminal::processCommand()
{
    string s(mCmdBuffer);
    Utils::trim(s);
    Utils::tokenize(s, ' ', mCmdTokens);
    printf2("Got command: %s\r\n", s.c_str());

    if ( mCmdTokens[0] == "set" && mCmdTokens.size() >= 2 ) {
        Event *e = EventPool::instance().newEvent(REQUEST_EVENT);
        e->request.operation = OP_SET;
        strncpy(e->request.field, mCmdTokens[1].c_str(), sizeof e->request.field);
        if ( mCmdTokens.size() > 2 )
          strncpy(e->request.value, mCmdTokens[2].c_str(), sizeof e->request.value);
        else
          e->request.value[0] = 0;

        EventQueue::instance().push(e);
    }
    else if ( mCmdTokens[0] == "get" && mCmdTokens.size() >= 2 ) {
        Event *e = EventPool::instance().newEvent(REQUEST_EVENT);
        e->request.operation = OP_GET;
        strncpy(e->request.field, mCmdTokens[1].c_str(), sizeof e->request.field);
        EventQueue::instance().push(e);
    }

}

void write_char(USART_TypeDef* USARTx, char c)
{
    while (!(USARTx->ISR & 0x00000040))
        ;

    USART_SendData(USARTx, c);
}

#ifdef MULTIPLEXED_OUTPUT

void DataTerminal::write(const char *cls, const char* s)
{
    write_char(USART2, '[');
    for ( size_t i = 0; i < strlen(cls); ++i )
        write_char(USART2, cls[i]);
    write_char(USART2, ']');

    for ( int i = 0; s[i] != 0; ++i )
        write_char(USART2, s[i]);

}

#else

void DataTerminal::write(const char* s, bool interactive)
{
    if ( mInteractive && !interactive )
        return;

    for ( int i = 0; s[i] != 0; ++i )
        write_char(USART2, s[i]);
}
#endif



void DataTerminal::_write(const char *s)
{
#ifdef MULTIPLEXED_OUTPUT
    write("", s);
#else
    write(s);
#endif
}


extern "C" {

void USART2_IRQHandler(void)
{
    if ( USART_GetITStatus(USART2, USART_IT_RXNE) ) {
        char c = (char) USART2->RDR; // This clears the interrupt right away
        DataTerminal::instance().processCharacter(c);
    }
}

}




/*
 * Terminal.cpp
 *
 *  Created on: Apr 19, 2016
 *      Author: peter
 */

#include "DataTerminal.hpp"

#include "stm32f30x.h"
#include "printf2.h"
#include "MenuScreens.hpp"
#include "Events.hpp"
#include <stdio.h>

#ifdef ENABLE_TERMINAL

DataTerminal &DataTerminal::instance()
{
    static DataTerminal __instance;
    return __instance;
}

void DataTerminal::init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE); // For USART3
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_7);

    // Initialize pins as alternative function 7 (USART)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_3;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);


    USART_InitTypeDef USART_InitStructure;
    USART_StructInit(&USART_InitStructure);

    USART_InitStructure.USART_BaudRate = 38400;
    USART_Init(USART3, &USART_InitStructure);
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;

    USART_Cmd(USART3, ENABLE);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_Init(&NVIC_InitStruct);
}

DataTerminal::DataTerminal()
    : mEscapes(0), mCurrentScreen(MAIN_SCREEN), mInteractive(false)
{
    EventQueue::instance().addObserver(this, CLOCK_EVENT|KEYPRESS_EVENT);
}

void DataTerminal::processEvent(Event *e)
{
    if ( e->type() == CLOCK_EVENT ) {
        ClockEvent *c = static_cast<ClockEvent*>(e);
        if ( c->mTime % 2 == 0 ) {
            mEscapes = 0;
        }
    }
    else if ( e->type() == KEYPRESS_EVENT ) {
        KeyPressEvent *k = static_cast<KeyPressEvent*>(e);
        processCharacter(k->key);
    }
}

void DataTerminal::processCharacter(char c)
{
    if ( !mInteractive )  {
        if ( c == 27 )
            ++mEscapes;

        if ( mEscapes == 3 ) {
            mInteractive = true;
            printf2("Entering interactive mode\r\n");
            showScreen(MAIN_SCREEN);
        }
    }
    else {
        if ( c == 13 ) {
            clearScreen();
            printf2("Exiting interactive mode\r\n");
            mInteractive = false;
        }
    }

}

void write_char(USART_TypeDef* USARTx, char c)
{
    while (!(USARTx->ISR & 0x00000040))
        ;

    USART_SendData(USARTx, c);
}

void DataTerminal::write(const char* s, bool interactive)
{
    if ( mInteractive && !interactive )
        return;

    for ( int i = 0; s[i] != 0; ++i )
        write_char(USART3, s[i]);
}

void DataTerminal::clearScreen()
{
    _write("\033[2J");
    _write("\033[H");
}

void DataTerminal::_write(const char *s)
{
    write(s, true);
}

void DataTerminal::showScreen(MenuScreen screen)
{
    char buff[100];
    switch(screen) {
        case MAIN_SCREEN: {
            clearScreen();
            //_write("******************************   Interactive Mode   ******************************\r\n");
            //_write("NOTICE: IT IS AGAINST USCG REGULATIONS FOR END USERS TO ALTER THEIR MMSI,\r\n");
            //_write("STATION NAME AND CALL SIGN AFTER INITIAL INSTALLATION.\r\n");
            //_write("IF YOUR UNIT HAS BEEN FACTORY PROGRAMMED, THIS TERMINAL WILL PROMPT YOU\r\n");
            //_write("FOR A SPECIAL CODE BEFORE ALLOWING THIS OPERATION. YOU CAN ONLY OBTAIN THE\r\n");
            //_write("CODE BY CONTACTING US AND PROVIDING THE UNIT'S SERIAL NUMBER AND CURRENT MMSI.\r\n\r\n");
            sprintf(buff, "Model: %s\r\n", MODEL);
            _write(buff);
            sprintf(buff, "Software revision: %s\r\n", REVISION);
            _write(buff);
#if 0
            sprintf(buff, "MMSI: %d\r\n", STATION_MMSI);
            _write(buff);
            sprintf(buff, "Name: %s\r\n", STATION_NAME);
            _write(buff);
            sprintf(buff, "Call sign: %s\r\n", STATION_CALLSIGN);
            _write(buff);
#endif
            _write("\r\n");
            _write("****************************** AIS Transponder Menu ******************************\r\n\r\n");
            _write("\t\033[1mP\033[0m\tProgram MMSI, station name and call sign \r\n");
            _write("\t\033[1mD\033[0m\tPerform diagnostics \r\n");
            _write("\t\033[1m<Enter>\033[0m\tReturn to AIS data mode\r\n\r\n");
            _write("**********************************************************************************\r\n\r\n");
            //_write("Note: The device will automatically return to AIS data mode if rebooted.\r\n");
            break;
        }
        case PROGRAMMING_SCREEN:
            break;
    }

    mCurrentScreen = screen;
}

extern "C" {

void USART3_IRQHandler(void)
{
    if ( USART_GetITStatus(USART3, USART_IT_RXNE) ) {
        char c = (char) USART3->RDR; // This clears the interrupt right away
        KeyPressEvent *e = static_cast<KeyPressEvent*>(EventPool::instance().newEvent(KEYPRESS_EVENT));
        if ( e == NULL )
            return;

        e->key = c;
        EventQueue::instance().push(e);
        //DataTerminal::instance().processCharacter(c);
    }
}

}

#endif




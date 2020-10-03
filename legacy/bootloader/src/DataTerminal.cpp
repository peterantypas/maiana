/*
 * Terminal.cpp
 *
 *  Created on: Apr 19, 2016
 *      Author: peter
 */

#include "DataTerminal.hpp"

#include "stm32f30x.h"
#include <stdio.h>
#include <cstring>
#include <diag/Trace.h>
#include <cassert>


//#define DRY_RUN


DataTerminal &DataTerminal::instance()
{
    static DataTerminal __instance;
    return __instance;
}

void DataTerminal::init()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE); // For USART3, LEDs
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_7);

    // Initialize pins as alternative function 7 (USART)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_1;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // These two pins are LEDs. Just turn them on to indicate we have entered "update" mode.
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    //GPIO_SetBits(GPIOB, GPIO_Pin_14);
    //GPIO_SetBits(GPIOB, GPIO_Pin_15);

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
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_Init(&NVIC_InitStruct);

    write("[DFU]Waiting for transfer\r\n");
}

DataTerminal::DataTerminal()
    : mState(STATE_WAITING), mByteCount(0), mWriteAddress(0)
{
    assert(APPLICATION_ADDRESS%FLASH_PAGE_SIZE == 0);
    assert(METADATA_ADDRESS%FLASH_PAGE_SIZE == 0);
    assert(sizeof mMetadata % 4 == 0);
    memset(mCurrPage, 0, sizeof mCurrPage);
}

void printmd5(uint8_t *d)
{
    char m[35];
    for ( size_t i = 0; i < 16; ++i ) {
        sprintf(m + i*2, "%.2x", d[i]);
    }
    trace_printf("%s\n",m);
}

void DataTerminal::processByte(uint8_t byte)
{
    switch(mState) {
        case STATE_WAITING:
            if ( byte == START_TRANSFER_CMD ) {
                mState = STATE_IN_TRANSFER_HEADER;
                mByteCount = 0;
                trace_printf("Receiving metadata\n");
                GPIO_SetBits(GPIOB, GPIO_Pin_14);
                writeCmd(ACK);
            }
            else {
                trace_printf("Bad command: %.2x\n", byte);
                fail();
            }
            break;
        case STATE_IN_TRANSFER_HEADER: {
            //trace_printf("1 byte\n");
            char *p = (char*)&mMetadata;
            memcpy(p + mByteCount, &byte, 1);
            ++mByteCount;
            if ( mByteCount == sizeof mMetadata ) {
                trace_printf("Magick: %.8x\n", mMetadata.magic);
                // Is the metadata sane?
                if ( mMetadata.magic == METADATA_MAGIC ) {
                    mState = STATE_IN_TRANSFER_BLOCK;
                    trace_printf("Expecting %d bytes\n", mMetadata.size);
                    printmd5(mMetadata.md5);
                    GPIO_SetBits(GPIOB, GPIO_Pin_14);
                    mByteCount = 0;
                    mWriteAddress = APPLICATION_ADDRESS;
                    MD5Init(&mMD5);
                    unlockFlash();
                    writeCmd(ACK);
                }
                else {
                    trace_printf("Bad metadata \n");
                    fail();
                }
            }
            }
            break;
        case STATE_IN_TRANSFER_BLOCK:
            MD5Update(&mMD5, &byte, 1);
            if ( mByteCount == 0 ) {
                GPIO_SetBits(GPIOB, GPIO_Pin_15);
            }

            size_t offset = mByteCount % FLASH_PAGE_SIZE;
            assert(offset < sizeof mCurrPage);
            memcpy(mCurrPage + offset, &byte, 1);

            ++mByteCount;

            if ( mByteCount % FLASH_PAGE_SIZE == 0 ) {
                flushPage();
            }

            // Acknowledge every 1K
            if ( mByteCount % 1024 == 0 ) {
                trace_printf("Sending ACK\n");
                writeCmd(ACK);
            }


            if ( mByteCount == mMetadata.size ) {
                if ( mMetadata.size % 1024 ) {
                    // We have a partial page to write
                    flushPage();
                    writeCmd(ACK);
                }

                // Last byte!
                trace_printf("Received %d bytes\n", mMetadata.size);
                GPIO_ResetBits(GPIOB, GPIO_Pin_14);
                mState = STATE_WAITING;

                uint8_t d[16];
                MD5Final(d, &mMD5);
                printmd5(d);
                if ( memcmp(d, mMetadata.md5, 16) ) {
                    // Bad MD5
                    trace_printf("MD5 mismatch :( \n");
                    fail();
                }
                else {
                    // Good transfer
                    trace_printf("MD5 match :) \n");
                    writeCmd(ACK);


                    flushMetadata();
                    lockFlash();
                    NVIC_SystemReset();
                }
            }
            break;
    }
}


void DataTerminal::fail()
{
    mByteCount = 0;
    mState = STATE_WAITING;
    GPIO_ResetBits(GPIOB, GPIO_Pin_14|GPIO_Pin_15);
    FLASH_Lock();
    trace_printf("Resetting\n");
    writeCmd(NACK);
}

void DataTerminal::unlockFlash()
{
#ifdef DRY_RUN
    trace_printf("Mock flash unlock\n");
#else
    FLASH_Unlock();
    FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
#endif
}

void DataTerminal::lockFlash()
{
#ifdef DRY_RUN
    trace_printf("Mock flash lock\n");
#else
    FLASH_Lock();
    FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
#endif
}

void DataTerminal::flushPage()
{
#ifdef DRY_RUN
    trace_printf("Mock write for flash page at %.8x\n", mWriteAddress);
    mWriteAddress += FLASH_PAGE_SIZE;
#else
    trace_printf("Writing Flash page at %.8x\n", mWriteAddress);
    FLASH_ErasePage(mWriteAddress);
    FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
    char *p = mCurrPage;
    for ( size_t i = 0; i < FLASH_PAGE_SIZE; i += 4, p += 4) {
        FLASH_ProgramWord(mWriteAddress + i, *(uint32_t*)p);
        FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
    }

    trace_printf("Wrote Flash page at %.8x\n", mWriteAddress);
    mWriteAddress += FLASH_PAGE_SIZE;
    memset(mCurrPage, 0, sizeof mCurrPage);
#endif
}

void DataTerminal::flushMetadata()
{
#ifdef DRY_RUN
    trace_printf("Mock write for metadata page at %.8x\n", METADATA_ADDRESS);
#else
    FLASH_ErasePage(METADATA_ADDRESS);
    FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
    char *p = (char*)&mMetadata;
    for ( size_t i = 0; i < sizeof mMetadata; i += 4, p += 4 ) {
        FLASH_ProgramWord(METADATA_ADDRESS + i, *(uint32_t*)p);
        FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
    }
#endif
}

void write_char(USART_TypeDef* USARTx, char c)
{
    while (!(USARTx->ISR & 0x00000040))
        ;

    USART_SendData(USARTx, c);
}

void DataTerminal::write(const char* s)
{
    for ( int i = 0; s[i] != 0; ++i )
        write_char(USART3, s[i]);
}

void DataTerminal::writeCmd(uint8_t cmd)
{
    write_char(USART3, (char)cmd);
}

void DataTerminal::clearScreen()
{
    _write("\033[2J");
    _write("\033[H");
}

void DataTerminal::_write(const char *s)
{
    write(s);
}


extern "C" {

void USART3_IRQHandler(void)
{
    if ( USART_GetITStatus(USART3, USART_IT_RXNE) ) {
        uint8_t byte = (uint8_t)USART3->RDR;
        DataTerminal::instance().processByte(byte);
    }
}

}






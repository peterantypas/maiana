/*
 * EEPROM.cpp
 *
 *  Created on: May 26, 2016
 *      Author: peter
 */

#include "EEPROM.hpp"
#include "printf2.h"
#include <stm32f30x_i2c.h>
#include "Utils.hpp"

#define EEPROM_STATION_ADDR         0x00    // A StationData structure starts here
#define EEPROM_REGION_CNT_ADDR      0x24    // Number of active special regions goes here (single byte)
#define EEPROM_REGION_ADDR          0x25    // Special region data array starts here


/*
 * The Microchip EEPROM doesn't really expect to share the I2C bus with any other EEPROM.
 * So the four high bits of the address are 1010 (same for most EEPROMs) and the lower 3 don't matter.
 */
#define EEPROM_ADDR (0x50 << 1)

EEPROM &EEPROM::instance()
{
    static EEPROM __instance;
    return __instance;
}


EEPROM::EEPROM()
{
    // TODO Auto-generated constructor stub

}

void EEPROM::init()
{
    //printf2("Size of station data: %d\r\n", sizeof(StationData));
    //printf2("Size of special region: %d\r\n", sizeof(SpecialRegion));

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    // We're using I2C1 via PB6 (SCL) and PB7 (SDA) as alternate function 4
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;  // Using the internal pull-up resistors for SDA/SCL. They seem to work fine.
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_Level_1;

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_4);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_4);

    GPIO_Init(GPIOB, &gpio);

    RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
    I2C_DeInit(I2C1);

    RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);  // Use the internal 8MHz clock for I2C, not the 72 MHz system clock
    I2C_InitTypeDef i2c;
    I2C_StructInit(&i2c);

    i2c.I2C_Mode = I2C_Mode_I2C;
    i2c.I2C_AnalogFilter = I2C_AnalogFilter_Disable;
    i2c.I2C_DigitalFilter = 0x00;
    i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    i2c.I2C_Ack = I2C_Ack_Enable;
    i2c.I2C_OwnAddress1 = 0x00;

    // Borrowed these settings from libopencm3 for the STM32F3 (https://github.com/libopencm3/libopencm3)
    i2c.I2C_Timing = (0x01 << 28) | (0x04 << 20) | (0x02 << 16) | (0xf << 8) | 0x13;

    I2C_Cmd(I2C1, DISABLE);
    I2C_Init(I2C1, &i2c);
    I2C_Cmd(I2C1, ENABLE);

}

uint8_t EEPROM::readByte(uint8_t addr)
{
    uint8_t result = 0;
    //printf2("EEPROM read waiting for non-busy status\r\n");

    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) == SET );

    I2C_TransferHandling(I2C1, EEPROM_ADDR, 1, I2C_SoftEnd_Mode, I2C_Generate_Start_Write);
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);

    //printf2("EEPROM read sending address\r\n");

    I2C_SendData(I2C1, (uint8_t)addr);

    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TC) == RESET);
    I2C_TransferHandling(I2C1, EEPROM_ADDR|1, 1, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);

    //printf2("EEPROM read waiting for RX register\r\n");
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET);

    result = I2C_ReceiveData(I2C1);

    //printf2("EEPROM read waiting for STOP flag\r\n");
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF) == RESET);

    I2C_ClearFlag(I2C1, I2C_FLAG_STOPF);

    return result;
}

void EEPROM::writeByte(uint8_t address, uint8_t byte)
{
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) == SET );

    I2C_TransferHandling(I2C1, EEPROM_ADDR, 1, I2C_Reload_Mode, I2C_Generate_Start_Write);
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);

    I2C_SendData(I2C1, (uint8_t)address);
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TCR) == RESET);

    I2C_TransferHandling(I2C1, EEPROM_ADDR, 1, I2C_AutoEnd_Mode, I2C_No_StartStop);
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);

    I2C_SendData(I2C1, byte);
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF) == RESET);

    I2C_ClearFlag(I2C1, I2C_FLAG_STOPF);

    // Should be about 5ms
    Utils::delay(360000);

}

void EEPROM::writeStationData(const StationData &data)
{
    uint8_t addr = EEPROM_STATION_ADDR;
    const uint8_t *p = (uint8_t*)&data;
    for ( size_t i = 0; i < sizeof data; ++i )
        writeByte(addr++, *p++);
}

void EEPROM::readStationData(StationData &data)
{
    uint8_t addr = EEPROM_STATION_ADDR;
    uint8_t *p = (uint8_t*)&data;
    for ( size_t i = 0; i < sizeof data; ++i ) {
        uint8_t byte = readByte(addr++);
        *p++ = byte;
    }
}





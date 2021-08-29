/*
 * bsp.hpp
 *
 *  Created on: Jun 28, 2021
 *      Author: peter
 */

#ifndef INC_BSP_BSP_HPP_
#define INC_BSP_BSP_HPP_

#include <stm32l4xx_hal.h>

// All HW initialization
void bsp_init();

// SPI
void bsp_mcp_2515_select();
void bsp_mcp_2515_unselect();
uint8_t bsp_spi_transfer(uint8_t byte);

// Arduino adapter
extern "C"
{
void delay(uint32_t ms);
}

void bsp_delay_us(uint32_t us);

void bsp_start_wdt();
void bsp_refresh_wdt();

// IRQ setup
bool bsp_is_isr();
typedef void(*can_irq_callback)();
void bsp_set_can_irq_cb(can_irq_callback cb);
typedef void(*uart_irq_callback)(char c);
void bsp_set_uart_irq_cb(uart_irq_callback cb);

// Persistence
uint8_t bsp_last_can_address();
void bsp_save_can_address(uint8_t);



#endif /* INC_BSP_BSP_HPP_ */

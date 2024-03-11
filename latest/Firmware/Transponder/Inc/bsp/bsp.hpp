/*
  Copyright (c) 2016-2020 Peter Antypas

  This file is part of the MAIANA™ transponder firmware.

  The firmware is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>
*/

#ifndef INC_BSP_HPP_
#define INC_BSP_HPP_

#include <stdint.h>
#include "config.h"
#include "StationData.h"
#include "ConfigFlags.h"

// See the bottom section for the proper BOARD_REV symbol format and either modify this header
// or define a different symbol in the preprocessor to build for a different board

#ifndef BOARD_REV
#define BOARD_REV 113
#endif

/**
 * These functions must be implemented for each supported board.
 */

void bsp_hw_init();
void bsp_write_char(const char c);
void bsp_write_string(const char *s);
void bsp_set_rx_mode();
void bsp_set_tx_mode();
void bsp_start_wdt();
void bsp_refresh_wdt();
uint32_t bsp_get_system_clock();
void bsp_reboot();
void bsp_enter_dfu();
void bsp_gnss_on();
void bsp_gnss_off();
bool bsp_is_tx_disabled();
bool bsp_is_tx_hardwired();
bool bsp_is_gnss_on();
void bsp_rx_led_on();
void bsp_rx_led_off();
void bsp_tx_led_on();
void bsp_tx_led_off();
void bsp_gps_led_on();
void bsp_gps_led_off();

/**
 * Station data persistence related
 */
void bsp_read_station_data(StationData *data);
void bsp_write_station_data(const StationData &data);
void bsp_erase_station_data();
bool bsp_is_station_data_provisioned();

/**
 * Configuration flag persistence
 */
void bsp_read_config_flags(ConfigFlags *flags);
void bsp_write_config_flags(const ConfigFlags &flags);
void bsp_erase_config_flags();

void bsp_read_xo_trim(XOTrim *t);
void bsp_write_xo_trim(const XOTrim &t);
void bsp_erase_xo_trim();

// Callback for processing UART input (interrupt)
typedef void(*char_input_cb)(char c);

// These callbacks must be installed
void bsp_set_gnss_input_callback(char_input_cb cb);
void bsp_set_terminal_input_callback(char_input_cb cb);

// Callback for GPIO and other interrupts
typedef void(*irq_callback)();

// These callbacks must be installed for the core AIS application
void bsp_set_gnss_1pps_callback(irq_callback cb);
void bsp_set_gnss_sotdma_timer_callback(irq_callback cb);
void bsp_set_trx_clk_callback(irq_callback cb);
void bsp_set_rx_clk_callback(irq_callback cb);
void bsp_set_tick_callback(irq_callback cb);

// Abstraction of the SOTDMA hardware timer
void bsp_start_sotdma_timer();
void bsp_stop_sotdma_timer();
uint32_t bsp_get_sotdma_timer_value();
void bsp_set_sotdma_timer_value(uint32_t v);

// Encapsulates the SPI bus
uint8_t bsp_tx_spi_byte(uint8_t b);



extern const char *BSP_HW_REV;

// BSP headers go here

#if BOARD_REV == 93
#include <bsp_9_3.hpp>
#elif BOARD_REV == 100
#include <bsp_10_0.hpp>
#elif BOARD_REV == 101
#include <bsp_10_1.hpp>
#elif BOARD_REV == 105
#include <bsp_10_5.hpp>
#elif BOARD_REV == 109
#include <bsp_10_9.hpp>
#elif BOARD_REV == 110
#include <bsp_11_0.hpp>
#elif BOARD_REV == 113
#include <bsp_11_3.hpp>
#endif







#endif /* INC_BSP_HPP_ */

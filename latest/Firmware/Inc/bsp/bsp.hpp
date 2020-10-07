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

#include "StationData.h"

// Current board revision is 5.0
// Either modify this header or define a different symbol in the preprocessor to build for a different board

#ifndef BOARD_REV
#define BOARD_REV 52
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

// Abstraction of the SOTDMA hardware timer
void bsp_start_sotdma_timer();
void bsp_stop_sotdma_timer();
uint32_t bsp_get_sotdma_timer_value();
void bsp_set_sotdma_timer_value(uint32_t v);

// Encapsulates the SPI bus
uint8_t bsp_tx_spi_byte(uint8_t b);

// Station data persistence support -- this is absolutely board specific (e.g. flash vs EEPROM)
bool bsp_erase_station_data();
bool bsp_save_station_data(const StationData &data);
bool bsp_read_station_data(StationData &data);

// Board-specific headers go here

#if BOARD_REV == 50
#include "bsp_5_0.hpp"
#elif BOARD_REV == 52
#include "bsp_5_2.hpp"
#elif BOARD_REV == 53
#include "bsp_5_3.hpp"
#elif BOARD_REV == 60
#include "bsp_6_0.hpp"
#endif







#endif /* INC_BSP_HPP_ */

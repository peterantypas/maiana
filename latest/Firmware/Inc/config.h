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


#ifndef CONFIG_H_
#define CONFIG_H_

#include "radio_config.h"
#include "TXPowerSettings.h"

// Define to enable FreeRTOS instead of "bare metal". Doesn't add much value at the moment.
//#define RTOS

/**
 * If this is defined, the device transmits carrier on channel 87 (161.975MHz) for 1 second after reset.
 */
//#define CALIBRATION_MODE               1

// Set to non-zero to enable transmission support
#define ENABLE_TX                       1


#ifdef CALIBRATION_MODE
#define TX_POWER_LEVEL              PWR_P33
#else
#ifdef TX_TEST_MODE
#define TX_POWER_LEVEL              PWR_P33
#else
#define TX_POWER_LEVEL              PWR_P33
#endif
#endif


/*
 * Defining this symbol forces all output (NMEA + debug) to a high-speed USART for tunneling to an application that demuxes it.
 * Multiplexed output is still ASCII, but every line of text begins with a message class in square brackets. Examples:
 *
 * [NMEA]!AIVDM,....
 * [DEBUG]GPS Initialized
 *
*/
//#define MULTIPLEXED_OUTPUT            1

#ifdef MULTIPLEXED_OUTPUT
#define DBG printf_serial
#else
#define DBG printf_null
#endif


// Some AIS messages can occupy 5 time slots. Nothing we care about exceeds 2 slots (2x256).
#define MAX_AIS_RX_PACKET_SIZE       512

// As a class B transponder, we never transmit anything bigger than 240 bits.
#define MAX_AIS_TX_PACKET_SIZE       256

// Maximum allowed backlog in TX queue
#define MAX_TX_PACKETS_IN_QUEUE        4

// Headroom above noise floor (in dB) that constitutes a clear channel for transmission
#define TX_CCA_HEADROOM                3

// Transmission intervals in seconds
#define MIN_TX_INTERVAL                5
#define MIN_MSG_18_TX_INTERVAL        30
#define MAX_MSG_18_TX_INTERVAL       180

// Default interval for message 24 A&B (static data report)
#define MSG_24_TX_INTERVAL           360

// The spec calls for Class B transmitters to listen for the first 20 bits of each frame before transmitting.
// It takes the Si4463 a few bits' time to switch from RX to TX, so I arbitrarily picked the 12th bit instead.
#define CCA_SLOT_BIT                  11

// Extra debugging using halting assertions
//#define DEV_MODE                       1

#define DFU_FLAG_ADDRESS              0x20009C00
#define DFU_FLAG_MAGIC                0xa191feed




#endif /* CONFIG_H_ */

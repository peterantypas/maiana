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


#define FW_REV                        "4.4.1"


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

// Set to true to emit proprietary NMEA sentences for debugging TX scheduling. Not useful in production.
#define REPORT_TX_SCHEDULING           0

// Set to true to force RSSI sampling at every SOTDMA timer slot on both channels
#define FULL_RSSI_SAMPLING             1

// Headroom above noise floor (in dB) that constitutes a clear channel for transmission
#if FULL_RSSI_SAMPLING
#define TX_CCA_HEADROOM                6
#else
// When sampling RSSI sparsely, there is a tendency to overestimate the noise floor, so there is no need for headroom
#define TX_CCA_HEADROOM                0
#endif


// Transmission intervals in seconds
#define MIN_TX_INTERVAL                5
#define MIN_MSG_18_TX_INTERVAL        30
#define MAX_MSG_18_TX_INTERVAL       180

// Default interval for message 24 A&B (static data report) = 6 minutes
#define MSG_24_TX_INTERVAL           360

// The spec calls for Class B transmitters to listen for the first 20 bits of each frame before transmitting.
// It takes the Si4463 a few bits' time to switch from RX to TX, so I arbitrarily picked the 12th bit instead.
#define CCA_SLOT_BIT                  11

// Extra debugging using halting assertions
//#define DEV_MODE                       1

#define BOOTMODE_ADDRESS              0x20009C00
#define DFU_FLAG_MAGIC                0xa191feed
#define CLI_FLAG_MAGIC                0x209a388d

#define OTP_DATA                      1

#define ENABLE_WDT                    1

/**
 * This is a bit of a pain, but in the legacy breakout boards the TX LED
 * was manipulated by both a hardware switch and the GPIO, so the MCU logic is reversed.
 *
 * The only way to support those legacy boards is to build a separate binary
 * with this switch predefined as 1. There is no automated way to detect which kind of breakout MAIANA has.
 */
#ifndef LEGACY_BREAKOUTS
#define LEGACY_BREAKOUTS              1
#endif



#endif /* CONFIG_H_ */

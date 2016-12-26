/*
 * defines.h
 *
 *  Created on: Dec 12, 2015
 *      Author: peter
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "radio_config.h"
#include "TXPowerSettings.h"

#define MODEL    "SEAWAIS_02"
#define REVISION "0.7"



/*
 * If this is defined, the device transmits low-power carrier on channel 86 (161.925MHz).
 * This is meant to be used for crystal calibration only
 */
//#define CALIBRATION_MODE               1

// Undefine to disable transmission
//#define ENABLE_TX                      1

#ifdef ENABLE_TX
//#define TX_TEST_MODE
#endif


#ifdef CALIBRATION_MODE
#define TX_POWER_LEVEL              PWR_P24
#else
#ifdef TX_TEST_MODE
#define TX_POWER_LEVEL              PWR_P33
#else
#define TX_POWER_LEVEL              PWR_P24
#endif
#endif


/*
 * Defining this symbol forces all output (NMEA + printf2) to a high-speed USART for tunneling to an application that demuxes it.
 * Multiplexed output is still ASCII, but every line of text begins with a message class in square brackets. Examples:
 *
 * [NMEA]!AIVDM,....
 * [DEBUG]GPS Initialized
 *
*/
#define MULTIPLEXED_OUTPUT            1

#define OUTPUT_GPS_NMEA
#define ENABLE_PRINTF2

// Some AIS messages can occupy 5 time slots. Nothing we care about exceeds 2 slots (2x256).
#define MAX_AIS_RX_PACKET_SIZE       512


#ifdef TX_TEST_MODE
// For testing, it's necessary to transmit longer packets for a basic RTL-SDR receiver to not "miss" them.
#define MAX_AIS_TX_PACKET_SIZE      1280
#else
// As a class B transponder, we never transmit anything bigger than 240 bits.
#define MAX_AIS_TX_PACKET_SIZE       256
#endif

// Maximum allowed backlog in TX queue
#define MAX_TX_PACKETS_IN_QUEUE        4

// Define this to set BT=0.4 for GMSK (as per ITU spec for AIS). Default is 0.5
#define USE_BT_04

// Transmission intervals
#define MIN_TX_INTERVAL                5
#define MIN_MSG_18_TX_INTERVAL        30
#define MAX_MSG_18_TX_INTERVAL       180

// Default interval for message 24 A&B (static data report) is 6 minutes
#define MSG_24_TX_INTERVAL           360

// The spec calls for Class B transmitters to listen for the first 20 bits of each frame before transmitting.
// It takes the Si4463 a few bits' time to switch from RX to TX, so I arbitrarily picked the 12th bit instead.
#define CCA_SLOT_BIT                  11

// Extra debugging using halting assertions
#define DEV_MODE                       1

// The bootloader lives at 0x08000000 (Flash start), and occupies  8K

#define FIRMWARE_METADATA_ADDRESS       0x08002400      // FW metadata lives @ 9K
#define APPLICATION_ADDRESS             0x08002800      // The actual firmware starts @ 10K (up to 110K)
#define CONFIGURATION_ADDRESS           0x0801E000      // General purpose configuration data lives @ 120K
#define STATION_DATA_ADDRESS            0x0801F000      // Station data (immutable in US per USCG regulation) lives @ 122K


#endif /* GLOBALS_H_ */

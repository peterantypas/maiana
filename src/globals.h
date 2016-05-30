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
#define REVISION "0.5.1"



/*
 * If this is defined, the device transmits low-power carrier at a nominal frequency of 162.075Mhz,
 * two channels above AIS. This is meant to be used for crystal calibration only
 */
//#define CALIBRATION_MODE               1

// Undefine to disable transmission
#define ENABLE_TX                      1

#ifdef ENABLE_TX
//#define TX_TEST_MODE
#endif


#ifdef CALIBRATION_MODE
#define TX_POWER_LEVEL              PWR_M27
#else
#ifdef TX_TEST_MODE
#define TX_POWER_LEVEL              PWR_P16
#else
#define TX_POWER_LEVEL              PWR_M10
#endif
#endif

#define ENABLE_TERMINAL
#define OUTPUT_GPS_NMEA
#define ENABLE_PRINTF2

// Some AIS messages can occupy 5 time slots (5x256 = 1280). We call it quits at 2 slots.
#define MAX_AIS_RX_PACKET_SIZE      512




#ifdef TX_TEST_MODE
// For testing, it's necessary to transmit longer packets for a basic RTL-SDR receiver to not "miss" them.
#define MAX_AIS_TX_PACKET_SIZE      1280
#else
// As a class B transponder, we never transmit anything bigger than 256 bits
#define MAX_AIS_TX_PACKET_SIZE       300
#endif

// Maximum allowed backlog in TX queue
#define MAX_TX_PACKETS_IN_QUEUE        4

// Define this to set BT=0.4 for GMSK (as per ITU spec for AIS). Default is 0.5
#define USE_BT_04

// Default interval for message 18 (position report)
#define DEFAULT_TX_INTERVAL           30

// Default interval for message 24 A&B (static data report)
#define MSG_24_TX_INTERVAL           180

#define CCA_SLOT_BIT                   8

// Extra debugging using halting assertions
#define DEV_MODE                       1




/*
 * Ideally, these should come from some non-volatile storage on board, but for now
 * we just define them here.
 *
 * NOTE: It is against US Coast Guard regulations to transmit AIS messages with an invalid MMSI. Consider yourself warned.
 * That said, just about everyone developing AIS equipment does this at some point, typically in lab environments.
 *
 * Also, manufacturers of AIS transmitters must program the MMSI, station name and other metadata on behalf of
 * their customers prior to shipping to a US address, as users are prohibited from altering those.
 *
 * Of course, regulations were written at a time when none would design and build their own AIS equipment in
 * an open-source fashion, so they are (as usual) at least a decade behind the technology curve.
 */

//#define STATION_MMSI                987654321
//#define STATION_NAME        "TEST STATION 01"
//#define STATION_CALLSIGN             "N0NNNN"


#endif /* GLOBALS_H_ */

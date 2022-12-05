#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
  WIFI_OPEN_AP,
  WIFI_SECURE_AP,
  WIFI_STATION
} wifi_operation_mode_t;

typedef enum
{
  NMEA_TCP_LISTENER,
  NMEA_TCP_SENDER,
  NMEA_UDP_SENDER
} nmea_gateway_mode_t;

void config_init();

/*** WiFi Configuration ***/

/**
 * @brief Returns the WiFi operation mode
 * 
 * @return wifi_operation_mode_t 
 */
wifi_operation_mode_t config_get_wifi_operation_mode();

/**
 * @brief Returns the SSID (regardless of mode)
 * 
 * @return const char* 
 */
const char *config_get_ssid();

/**
 * @brief Returns the password (if applicable)
 * 
 * @return const char* 
 */
const char *config_get_password();

/**
 * @brief Configures the WiFi interface. Will not take effect until reboot.
 * 
 * @param mode Desired mode
 * @param ssid Desired SSID 
 * @param password Desired password. May be NULL if not applicable.
 */
void config_wifi(wifi_operation_mode_t mode, const char *ssid, const char *password);

/*** NMEA configuration ***/

/**
 * @brief Returns NMEA gateway mode
 * 
 * @return nmea_gateway_mode_t 
 */
nmea_gateway_mode_t config_get_nmea_gateway_mode();

/**
 * @brief Returns the IP address (not applicable for NMEA_TCP_LISTENER)
 * 
 * @return const char* 
 */
const char *config_get_nmea_gateway_ip();

/**
 * @brief Returns the TCP port (applicable to all modes)
 * 
 * @return uint16_t 
 */
uint16_t config_get_nmea_gateway_port();

/**
 * @brief Indicates whether the NMEA output stream includes GNSS sentences
 * 
 * @return true 
 * @return false 
 */
bool config_nmea_stream_includes_gnss();

/**
 * @brief Congigures the NMEA gateway
 * 
 * @param mode Desired mode
 * @param ip Target IP (for senders only)
 * @param port TCP port
 * @param include_gnss Whether or not to include GNSS sentences
 */
void config_nmea_gateway(nmea_gateway_mode_t mode, const char *ip, uint16_t port, bool include_gnss);

/*** Misc ***/

/**
 * @brief Returns the AP MAC address
 * 
 * @return const char* 
 */
const char *config_ap_mac_address();

/**
 * @brief Returns the station MAC address
 * 
 * @return const char* 
 */
const char *config_st_mac_address();

/**
 * @brief Resets everything to defaults. Will take effect after reboot.
 * 
 */
void config_reset_all();


#endif

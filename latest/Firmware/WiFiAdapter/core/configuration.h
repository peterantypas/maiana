#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include <string.h>

typedef enum
{
  WIFI_OPEN_AP,
  WIFI_SECURE_AP,
  WIFI_STATION
} wifi_operation_mode_t;

wifi_operation_mode_t config_get_wifi_operation_mode();

const char *config_get_ssid();

const char *config_get_password();

const char *config_ap_mac_address();

const char *config_st_mac_address();


#endif

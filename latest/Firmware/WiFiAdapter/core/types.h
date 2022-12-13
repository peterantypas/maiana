#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <esp_event.h>

typedef enum
{
  WIFI_OPEN_AP,
  WIFI_SECURE_AP,
  WIFI_STATION
}
wifi_operation_mode_t;

typedef enum
{
  NMEA_TCP_LISTENER,
  NMEA_UDP_SENDER
} 
nmea_gateway_mode_t;

typedef struct 
{
  char text[128];
} 
serial_message_t;

typedef void (nmea_data_callback_t)(const char *text);

typedef struct
{
  int64_t mmsi;
  char name[24];
  char callsign[8];
  int32_t type;
  int16_t len;
  int16_t beam;
  int16_t port_offs;
  int16_t bow_offs;
}
ais_station_t;

typedef struct
{
  char hw[32];
  char fw[32];
}
ais_system_t;


ESP_EVENT_DECLARE_BASE(MAIANA_EVENT);

#define BTN_EVENT                   1
#define ONE_SEC_TIMER_EVENT         2
#define REBOOT_EVENT                3
#define NMEA_RESTART_EVENT          4

#endif

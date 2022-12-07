#ifndef __TYPES_H__
#define __TYPES_H__

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
  NMEA_TCP_SENDER,
  NMEA_UDP_SENDER
} 
nmea_gateway_mode_t;

typedef struct 
{
  char text[128];
} 
serial_message_t;

ESP_EVENT_DECLARE_BASE(MAIANA_EVENT);

#define BTN_EVENT                   1
#define ONE_SEC_TIMER_EVENT         2


#endif

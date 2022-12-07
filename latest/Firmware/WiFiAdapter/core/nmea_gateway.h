#ifndef __NMEA_GATEWAY_H__
#define __NMEA_GATEWAY_H__

#include <stdint.h>
#include "configuration.h"
#include <esp_event.h>

ESP_EVENT_DECLARE_BASE(NMEA_EVENT);

#define NMEA_GNSS_SENTENCE    1
#define NMEA_AIS_SENTENCE     2
#define NMEA_PROPR_SENTENCE   3

void nmea_gateway_start();




#endif

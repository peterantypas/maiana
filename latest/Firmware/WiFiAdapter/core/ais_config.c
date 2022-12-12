#include "ais_config.h"
#include "types.h"
#include "nmea_gateway.h"
#include <string.h>
#include <esp_log.h>
#include <stdio.h>
#include "utils.h"

typedef enum 
{
  NMEA_SCAN_NONE,
  NMEA_SCAN_STATION,
  NMEA_SCAN_SYS,
  NMEA_SCAN_CLI
}
nmea_scan_state_t;

static const char *TAG = "ais";
static nmea_scan_state_t __scan_state = NMEA_SCAN_NONE;
static char __sys[128] = {0};
static char __station[128] = {0};


static void nmea_sentence_callback(const char *text)
{
  //printf(text);
  switch(__scan_state)
  {
    case NMEA_SCAN_STATION:
    {
      if ( strstr(text, "$PAISTN") )
      {
        strncpy(__station, text, sizeof __station);
        //ESP_LOGI(TAG, "%s", __station);        
        __scan_state = NMEA_SCAN_NONE;
      }
      break;
    }
    case NMEA_SCAN_SYS:
    {
      if ( strstr(text, "$PAISYS") )
      {
        strncpy(__sys, text, sizeof __sys);
        ESP_LOGI(TAG, "%s", __sys);
        __scan_state = NMEA_SCAN_NONE;
      }
      break;
    }
    case NMEA_SCAN_CLI:
    {
      break;
    }
    default:
      break;
  }
}


void ais_config_init()
{
  nmea_gateway_set_callback(nmea_sentence_callback);
}

bool ais_config_read(ais_station_t *data)
{
  __scan_state = NMEA_SCAN_STATION;
  nmea_gateway_send_command("station?\r\n");
  for ( int i = 0; i < 20; ++i )
  {
    if ( __scan_state == NMEA_SCAN_NONE )
      {
        memset(data, 0, sizeof(ais_station_t));
        char *index[20];
        int count = tokenize(__station, ",*", index, 20);
        if ( count >= 10 )
        {
          data->mmsi = strtoll(index[1], NULL, 10);
          strncpy(data->name, index[2], sizeof data->name);
          strncpy(data->callsign, index[3], sizeof data->callsign);
          data->type = atoi(index[4]);
          data->len = atoi(index[5]);
          data->beam = atoi(index[6]);
          data->port_offs = atoi(index[7]);
          data->bow_offs = atoi(index[8]);
          ESP_LOGI(TAG, "%lld, %s, %s, %d, %d, %d, %d, %d", data->mmsi, data->name, data->callsign, data->type, data->len, data->beam, data->port_offs, data->bow_offs);
          return true;
        }
        else
          return false;
      }
    usleep(100000);
  }

  ESP_LOGI(TAG, "Did not read station data");
  return false;
}

bool ais_config_write(ais_station_t *data)
{
  return false;
}




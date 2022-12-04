#include "configuration.h"
#include "nvs_flash.h"
#include "esp_mac.h"

static char __ssid[48] = {0};
static char __ap_mac[32] = {0};
static char __st_mac[32] = {0};

const char *config_ap_mac_address()
{
  if ( __ap_mac[0] == 0 )
  {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
    sprintf(__ap_mac, "%.2X%.2X%.2X%.2X%.2X%.2X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  }

  return __ap_mac;
}

const char *config_st_mac_address()
{
  if ( __st_mac[0] == 0 )
  {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    sprintf(__st_mac, "%.2X%.2X%.2X%.2X%.2X%.2X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  }

  return __st_mac;
}

wifi_operation_mode_t config_get_wifi_operation_mode()
{
  return WIFI_OPEN_AP;
}

const char *config_get_ssid()
{
  switch(config_get_wifi_operation_mode())
  {
    case WIFI_OPEN_AP:
    case WIFI_SECURE_AP:
      if ( __ssid[0] == 0 )
      {
        const char *mac = config_ap_mac_address();
        sprintf(__ssid, "MAIANA_%s", mac+6);
      }
      break;
    case WIFI_STATION:
      if ( __ssid[0] == 0 )
      {
        const char *mac = config_st_mac_address();
        sprintf(__ssid, "MAIANA_%s", mac+6);
      }
      break;
    default:
      return NULL;
  }

  return __ssid;
}

const char *config_get_password()
{
  return NULL;
}


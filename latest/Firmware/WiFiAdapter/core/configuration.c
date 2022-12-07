#include "configuration.h"
#include "nvs_flash.h"
#include "esp_mac.h"

ESP_EVENT_DEFINE_BASE(CONFIG_EVENT);


static char __ssid[48] = {0};
static char __password[60] = {0};
static char __ap_mac[32] = {0};
static char __st_mac[32] = {0};
static char __nmea_ip[16] = {0};
static nvs_handle_t __nvs = 0;

#define NMEA_DEFAULT_PORT     5555

// NVS Keys
#define WIFI_MODE_KEY         "wifi_mode"
#define WIFI_SSID_KEY         "wifi_ssid"
#define WIFI_PASSWORD_KEY     "wifi_pwd"

#define NMEA_MODE_KEY         "nmea_mode"
#define NMEA_IP_ADDR_KEY      "nmea_ip"
#define NMEA_PORT_KEY         "nmea_port"


void config_init()
{
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
  {
    nvs_flash_erase();
    nvs_flash_init();
  }
  nvs_open("maiana", NVS_READWRITE, &__nvs);
}

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

void config_reset_all()
{
  nvs_erase_all(__nvs);
  nvs_commit(__nvs);
}

////////////////////////////////////////////////////////////////////////////////////
// WiFi
////////////////////////////////////////////////////////////////////////////////////

const char *config_get_ssid()
{
  size_t len = sizeof __ssid;

  switch(config_get_wifi_operation_mode())
  {
    case WIFI_OPEN_AP:
    case WIFI_SECURE_AP:
      if ( nvs_get_str(__nvs, WIFI_SSID_KEY, __ssid, &len) == ESP_ERR_NVS_NOT_FOUND || __ssid[0] == 0 )
      {
        const char *mac = config_ap_mac_address();
        sprintf(__ssid, "MAIANA_%s", mac+6);
      }
      break;
    case WIFI_STATION:
      if ( nvs_get_str(__nvs, WIFI_SSID_KEY, __ssid, &len) == ESP_ERR_NVS_NOT_FOUND || __ssid[0] == 0 )
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
  size_t len = sizeof __password;

  if ( nvs_get_str(__nvs, WIFI_PASSWORD_KEY, __password, &len) == ESP_ERR_NVS_NOT_FOUND )
    return NULL;

  return __password;
}

wifi_operation_mode_t config_get_wifi_operation_mode()
{
  int mode = -1;
  if ( nvs_get_i32(__nvs, WIFI_MODE_KEY, &mode) == ESP_ERR_NVS_NOT_FOUND )
    return WIFI_OPEN_AP;

  return (wifi_operation_mode_t)mode;
}

void config_wifi(wifi_operation_mode_t mode, const char *ssid, const char *password)
{
  nvs_set_i32(__nvs, WIFI_MODE_KEY, mode);
  nvs_set_str(__nvs, WIFI_SSID_KEY, ssid);
  if ( password )
    nvs_set_str(__nvs, WIFI_PASSWORD_KEY, password);

  nvs_commit(__nvs);
  esp_event_post(CONFIG_EVENT, WIFI_CONFIG_CHANGED_EVENT, NULL, 0, portMAX_DELAY);
}

////////////////////////////////////////////////////////////////////////////////////
// NMEA
////////////////////////////////////////////////////////////////////////////////////
nmea_gateway_mode_t config_get_nmea_gateway_mode()
{
  nmea_gateway_mode_t result;
  if ( nvs_get_i32(__nvs, NMEA_MODE_KEY, (int*)&result) == ESP_ERR_NVS_NOT_FOUND )
    return NMEA_TCP_LISTENER;
  
  return result;
}

const char *config_get_nmea_gateway_ip()
{
  size_t len = sizeof __nmea_ip;
  if ( config_get_nmea_gateway_mode() == NMEA_TCP_LISTENER )
    return "0.0.0.0";
  else
    nvs_get_str(__nvs, NMEA_IP_ADDR_KEY, __nmea_ip, &len);

  return __nmea_ip;
}

uint16_t config_get_nmea_gateway_port()
{
  uint16_t result;
  if ( nvs_get_u16(__nvs, NMEA_PORT_KEY, &result) == ESP_ERR_NVS_NOT_FOUND )
    return NMEA_DEFAULT_PORT;

  return result;
}

void config_nmea_gateway(nmea_gateway_mode_t mode, const char *ip, uint16_t port)
{
  switch(mode)
  {
    case NMEA_TCP_LISTENER:
      // IP is always "0.0.0.0"
      nvs_set_str(__nvs, NMEA_IP_ADDR_KEY, "0.0.0.0");
      nvs_set_u16(__nvs, NMEA_PORT_KEY, port);
      nvs_set_i32(__nvs, NMEA_MODE_KEY, mode);
      break;
    case NMEA_TCP_SENDER:
    case NMEA_UDP_SENDER:
      nvs_set_str(__nvs, NMEA_IP_ADDR_KEY, ip);
      nvs_set_u16(__nvs, NMEA_PORT_KEY, port);
      nvs_set_i32(__nvs, NMEA_MODE_KEY, mode);
      break;
  }

  nvs_commit(__nvs);
  esp_event_post(CONFIG_EVENT, NMEA_CONFIG_CHANGED_EVENT, NULL, 0, portMAX_DELAY);

}


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "wifi.h"
#include "configuration.h"

static const char *TAG = "wifi";


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
  switch(event_id)
  {
    case WIFI_EVENT_AP_STACONNECTED:
    {
      wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
      ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                MAC2STR(event->mac), event->aid);
      break;
    }
    case WIFI_EVENT_AP_STADISCONNECTED:
    {
      wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
      ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                MAC2STR(event->mac), event->aid);
      break;
    }
    case WIFI_EVENT_STA_START:
    {
      esp_wifi_connect();
      break;
    }
    case WIFI_EVENT_STA_CONNECTED:
    {
      break;
    }
    case WIFI_EVENT_STA_DISCONNECTED:
    {
      esp_wifi_connect();
      break;
    }
    default:
      break;
  }

}

void wifi_start()
{
    esp_netif_init();
    wifi_operation_mode_t mode = config_get_wifi_operation_mode();

    if ( mode != WIFI_STATION )
      esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {0};
    const char *ssid = config_get_ssid();
    const char *password = config_get_password();

    switch(mode)
    {
      case WIFI_OPEN_AP:
      {
        memcpy(wifi_config.ap.ssid, ssid, strlen(ssid));
        wifi_config.ap.ssid_len = strlen(ssid);
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        wifi_config.ap.max_connection = 8;
        esp_wifi_set_mode(WIFI_MODE_AP);
        esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
      }
      break;
      case WIFI_SECURE_AP:
      {
        memcpy(wifi_config.ap.ssid, ssid, strlen(ssid));
        wifi_config.ap.ssid_len = strlen(ssid);
        strncpy((char *)wifi_config.ap.password, password, sizeof wifi_config.ap.password);
      
        wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
        wifi_config.ap.max_connection = 8;
        esp_wifi_set_mode(WIFI_MODE_AP);
        esp_wifi_set_config(WIFI_IF_AP, &wifi_config);

      } 
      break;
      case WIFI_STATION:
      {
        memcpy(wifi_config.sta.ssid, ssid, strlen(ssid));
        strncpy((char *)wifi_config.sta.password, password, sizeof wifi_config.ap.password);
        esp_wifi_set_mode(WIFI_MODE_STA);
        esp_wifi_set_config(WIFI_IF_STA, &wifi_config);        
      }
      break;         
    }

    esp_wifi_start();
}

#include <stdio.h>
#include "esp_event.h"
#include "bsp.h"
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi.h"
#include "http_server.h"
#include "configuration.h"
#include "button.h"
#include "types.h"
#include "nmea_gateway.h"
#include "ais_config.h"


void one_sec_timer()
{
  esp_event_isr_post(MAIANA_EVENT, ONE_SEC_TIMER_EVENT, NULL, 0, NULL);
}

static int sec_count = 0;
static float voltage = 0.0f;
static float alpha = 0.1;

static void one_sec_handler(void *args, esp_event_base_t base, int32_t id, void *data)
{
  float v = bsp_read_vbat() / 1000.0f;
  if ( sec_count == 0 )
    voltage = v;
  else
    voltage = alpha * v + (1.0 - alpha)*v;

  ++sec_count;

  if ( sec_count % 10 == 0 )
    ESP_LOGI("main", "Battery voltage: %.3f V", voltage);
}

void reboot_handler(void *args, esp_event_base_t base, int32_t id, void *data)
{
  sleep(2);
  bsp_reboot();
}

void app_main(void)
{
  esp_event_loop_create_default();
  esp_event_handler_register(MAIANA_EVENT, REBOOT_EVENT, reboot_handler, NULL); 
  esp_event_handler_register(MAIANA_EVENT, ONE_SEC_TIMER_EVENT, one_sec_handler, NULL); 
  

  //sleep(5);
  config_init();
  //config_reset_all();

  bsp_set_timer_cb(one_sec_timer);
  bsp_hw_init();

  //bsp_read_vbat();
  wifi_start();
  start_httpd();
  //button_init();

  nmea_gateway_start();

  sleep(1);
  ais_config_init();
  //ais_station_t s;
  //ais_config_read(&s);

  vTaskDelete(NULL);
}

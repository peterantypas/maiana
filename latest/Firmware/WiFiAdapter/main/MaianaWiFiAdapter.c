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



void one_sec_timer()
{
  esp_event_isr_post(MAIANA_EVENT, ONE_SEC_TIMER_EVENT, NULL, 0, NULL);
}


void app_main(void)
{
  esp_event_loop_create_default();

  config_init();
  //config_reset_all();

  bsp_set_timer_cb(one_sec_timer);
  bsp_hw_init();


  wifi_start();
  start_httpd();
  button_init();
  nmea_gateway_start();

  vTaskDelete(NULL);
}

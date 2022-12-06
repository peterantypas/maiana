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


void uart_rx_cb(char *s)
{
  if ( strlen(s) == 0 )
    return;

  printf(s);
}




void app_main(void)
{
  esp_event_loop_create_default();

  config_init();
  //config_reset_all();

  bsp_set_uart_rx_cb(uart_rx_cb);
  bsp_hw_init();

  wifi_start();
  start_httpd();
  button_init();

  vTaskDelete(NULL);
}

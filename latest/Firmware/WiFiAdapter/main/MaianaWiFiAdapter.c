#include <stdio.h>
#include "esp_event.h"
#include "bsp.h"
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi.h"
#include "http_server.h"
#include "configuration.h"


void uart_rx_cb(char *s)
{
  if ( strlen(s) == 0 )
    return;

  printf(s);
}

void btn_press_handler(void *args, esp_event_base_t base, int32_t id, void *data)
{
  static int last_tick = 0;
  int this_tick = xTaskGetTickCount();
  if ( this_tick - last_tick > 250 / portTICK_PERIOD_MS )
  {
    ESP_LOGI("main", "TX_EN button toggled");
    last_tick = this_tick;
  }
}



void app_main(void)
{
  esp_event_loop_create_default();

  config_init();
  //config_reset_all();

  bsp_set_uart_rx_cb(uart_rx_cb);
  bsp_hw_init();

  esp_event_handler_register(BSP_EVENT, BSP_TX_BTN_EVENT, btn_press_handler, NULL);
  wifi_start();

  start_httpd();

  vTaskDelete(NULL);
}

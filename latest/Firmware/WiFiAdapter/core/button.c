#include "esp_event.h"
#include <string.h>
#include "esp_log.h"
#include "../bsp/bsp.h"
#include "configuration.h"
#include "types.h"

const char *TAG = "button";
static int toggles = 0;
static uint32_t last_btn_press = 0;

void button_isr()
{
  esp_event_isr_post(MAIANA_EVENT, BTN_EVENT, NULL, 0, NULL);
}


void btn_press_handler(void *args, esp_event_base_t base, int32_t id, void *data)
{
  static int last_tick = 0;
  uint32_t this_tick = xTaskGetTickCount();

  if ( this_tick - last_tick > 250 / portTICK_PERIOD_MS )
  {
    ESP_LOGI(TAG, "TX_EN button toggled");
    last_tick = this_tick;
    last_btn_press = this_tick;
    ++toggles;
  }

  if ( toggles > 4 )
  {
    config_reset_all();
    bsp_reboot();
  }
}

static void one_sec_handler(void *args, esp_event_base_t base, int32_t id, void *data)
{
  uint32_t this_tick = xTaskGetTickCount();
  if ( toggles && (this_tick - last_btn_press > 5000/portTICK_PERIOD_MS) )
  {
    ESP_LOGI(TAG, "Disarmed toggle button logic");
    toggles = 0;
  }
}

void button_init()
{
  esp_event_handler_register(MAIANA_EVENT, BTN_EVENT, btn_press_handler, NULL); 
  esp_event_handler_register(MAIANA_EVENT, ONE_SEC_TIMER_EVENT, one_sec_handler, NULL); 
  bsp_set_button_isr_cb(button_isr);
}

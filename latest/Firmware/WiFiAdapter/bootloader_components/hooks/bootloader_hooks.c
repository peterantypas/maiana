#include <hal/gpio_hal.h>
#include <unistd.h>
#include <esp_log.h>

#define GPIO_RED_LED          GPIO_NUM_5
#define GPIO_GREEN_LED        GPIO_NUM_4


void bootloader_hooks_include(void)
{
}

void bootloader_before_init(void) 
{
  ESP_LOGI("hook", "Custom hook (before init)");
}

void bootloader_after_init(void) 
{

  /**
   * @brief Turn on both LEDs immediately. This way, if firmware is hosed and can't run, 
   * at least there's an indication that the system is on.
   * 
   */

  ESP_LOGI("hook", "Custom hook (after init)");
  gpio_ll_output_enable(&GPIO, GPIO_GREEN_LED);
  gpio_ll_output_enable(&GPIO, GPIO_RED_LED);

  gpio_ll_set_level(&GPIO, GPIO_GREEN_LED, 1);
  gpio_ll_set_level(&GPIO, GPIO_RED_LED, 1);

  //sleep(1);
}

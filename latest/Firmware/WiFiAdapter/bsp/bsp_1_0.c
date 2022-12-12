#include "bsp.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "hal/gpio_hal.h"
#include <string.h>
#include "freertos/timers.h"
#include "esp_intr_alloc.h"

static uart_rx_cb_t *uart_cb = NULL;
static button_isr_cb_t *btn_cb = NULL;
static timer_cb_t *timer_cb = NULL;
static TimerHandle_t timer_h;
static StaticTimer_t static_timer;

#define BUF_SIZE (128)
#define RD_BUF_SIZE (BUF_SIZE)
uint8_t dtmp[RD_BUF_SIZE];

#define GPIO_UART1_RX   GPIO_NUM_25
#define GPIO_UART1_TX   GPIO_NUM_26
#define GPIO_TX_BUTTON  GPIO_NUM_34

void uart_rx_task(void *params)
{
  char c;
  while (true)
  {
    if ( uart_read_bytes(UART_NUM_1, &c, 1, portMAX_DELAY) == 1 && uart_cb )
    {
      (*uart_cb)(c);
    }
  }
}

void bsp_uart_write(const char *text)
{
  uart_write_bytes(UART_NUM_1, text, strlen(text));
}

void bsp_uart_init()
{
  uart_config_t uart_config = {
    .baud_rate = 38400,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
  };
  uart_param_config(UART_NUM_1, &uart_config);
  uart_set_pin(UART_NUM_1, GPIO_UART1_TX, GPIO_UART1_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  uart_driver_install(UART_NUM_1, BUF_SIZE*2, 0, 0, NULL, 0);
  uart_enable_rx_intr(UART_NUM_1);
  //uart_enable_tx_intr(UART_NUM_1, 1, 0);

  xTaskCreate(uart_rx_task, "uart_event_task", 1024, NULL, 5, NULL);
}

void bsp_set_uart_rx_cb(uart_rx_cb_t *cb)
{
  uart_cb = cb;
}

void bsp_set_button_isr_cb(button_isr_cb_t *cb)
{
  btn_cb = cb;
}

void bsp_set_timer_cb(timer_cb_t *cb)
{
  timer_cb = cb;
}


void config_gpio(uint32_t key_gpio_pin, gpio_mode_t gpio_mode, bool pullup, bool pulldown)
{
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.pin_bit_mask = (uint64_t) 1 << key_gpio_pin;
  io_conf.mode = gpio_mode;
  io_conf.pull_up_en = pullup;
  io_conf.pull_down_en = pulldown;
  gpio_config(&io_conf);
}

void btn_isr(void *args)
{
  if ( btn_cb )
    (*btn_cb)();
}

void bsp_timer_tick()
{
  if ( timer_cb )
    (*timer_cb)();

}

void bsp_timer_init()
{
  timer_h = xTimerCreateStatic("OneSec", 1000 / portTICK_PERIOD_MS, pdTRUE, (void*)0, bsp_timer_tick, &static_timer);
  if ( timer_h )
    xTimerStart(timer_h, 0);
}


void bsp_hw_init()
{
  bsp_uart_init();

  config_gpio(GPIO_TX_BUTTON, GPIO_MODE_INPUT, false, false);
  gpio_set_direction(GPIO_TX_BUTTON, GPIO_MODE_INPUT);
  gpio_set_intr_type(GPIO_TX_BUTTON, GPIO_INTR_NEGEDGE);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(GPIO_TX_BUTTON, btn_isr, NULL);
  bsp_timer_init();
}

void bsp_reboot()
{
  esp_restart(); 
}


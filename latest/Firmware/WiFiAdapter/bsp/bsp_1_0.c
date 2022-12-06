#include "bsp.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "hal/gpio_hal.h"
#include <string.h>
#include "freertos/timers.h"

//static const char *TAG = "bsp";
static QueueHandle_t uart1_queue;
static uart_rx_cb_t *uart_cb = NULL;
static TimerHandle_t timer_h;
static StaticTimer_t static_timer;

#define BUF_SIZE (128)
#define RD_BUF_SIZE (BUF_SIZE)
uint8_t dtmp[RD_BUF_SIZE];

#define GPIO_UART1_RX   GPIO_NUM_25
#define GPIO_UART1_TX   GPIO_NUM_26
#define GPIO_TX_BUTTON  GPIO_NUM_34


static void uart_event_task(void *pvParameters)
{
  uart_event_t event;
  for(;;)
  {
    if(xQueueReceive(uart1_queue, (void * )&event, (TickType_t)portMAX_DELAY))
    {
      bzero(dtmp, RD_BUF_SIZE);
      switch(event.type)
      {
        case UART_DATA:
          uart_read_bytes(UART_NUM_1, dtmp, event.size, portMAX_DELAY);
          if ( uart_cb )
          {
            (*uart_cb)((char*)dtmp);
          }
          break;
        //Event of HW FIFO overflow detected                                                                     
        case UART_FIFO_OVF:
          // If fifo overflow happened, you should consider adding flow control for your application.            
          // The ISR has already reset the rx FIFO,                                                              
          // As an example, we directly flush the rx buffer here in order to read more data.                     
          uart_flush_input(UART_NUM_1);
          xQueueReset(uart1_queue);
          break;
	      //Event of UART ring buffer full                                                                         
	      case UART_BUFFER_FULL:
          // If buffer full happened, you should consider encreasing your buffer size                            
          // As an example, we directly flush the rx buffer here in order to read more data.                     
          uart_flush_input(UART_NUM_1);
          xQueueReset(uart1_queue);
          break;
        default:
          break;
      }
    }
  }
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
  uart_driver_install(UART_NUM_1, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart1_queue, 0);
  uart_param_config(UART_NUM_1, &uart_config);
  uart_enable_rx_intr(UART_NUM_1);

  xTaskCreate(uart_event_task, "uart_event_task", 4096, NULL, 5, NULL);
}

void bsp_set_uart_rx_cb(uart_rx_cb_t *cb)
{
  uart_cb = cb;
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
  esp_event_isr_post(BSP_EVENT, BSP_TX_BTN_EVENT, NULL, 0, NULL);
}

void bsp_timer_tick()
{
  esp_event_isr_post(BSP_EVENT, BSP_ONE_SEC_TIMER_EVENT, NULL, 0, NULL);
}

void bsp_timer_init()
{
  timer_h = xTimerCreateStatic("OneSec", 1000 / portTICK_PERIOD_MS, pdTRUE, (void*)0, bsp_timer_tick, &static_timer);
  if ( timer_h )
    xTimerStart(timer_h, 0);
}


void bsp_hw_init()
{
  uart_set_pin(UART_NUM_1, GPIO_UART1_TX, GPIO_UART1_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
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

ESP_EVENT_DEFINE_BASE(BSP_EVENT);

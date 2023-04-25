#include "bsp.h"
#include "nvs_flash.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "hal/gpio_hal.h"
#include <string.h>
#include "freertos/timers.h"
#include "esp_intr_alloc.h"
#include <esp_log.h>

#define TAG "bsp"

static uart_rx_cb_t *uart_cb = NULL;
static button_isr_cb_t *btn_cb = NULL;
static timer_cb_t *timer_cb = NULL;
static TimerHandle_t timer_h;
static StaticTimer_t static_timer;

#define BUF_SIZE (128)
#define RD_BUF_SIZE (BUF_SIZE)
uint8_t dtmp[RD_BUF_SIZE];

#define GPIO_UART1_RX       GPIO_NUM_25
#define GPIO_UART1_TX       GPIO_NUM_26
#define GPIO_TX_BUTTON      GPIO_NUM_34
#define GPIO_RED_LED        GPIO_NUM_5
#define GPIO_GREEN_LED      GPIO_NUM_4
#define GPIO_STM32_RESET_N  GPIO_NUM_12
#define GPIO_STM32_TX_EN    GPIO_NUM_13
#define I2C_MASTER_SCL_IO   GPIO_NUM_22               
#define I2C_MASTER_SDA_IO   GPIO_NUM_21               
#define I2C_MASTER_FREQ_HZ  100000                   
#define I2C_ADC_ADDRESS     0x48

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

  xTaskCreate(uart_rx_task, "uart_event_task", 4096, NULL, 5, NULL);
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

static esp_err_t i2c_master_init(void)
{
  i2c_config_t conf = 
  {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_MASTER_SDA_IO,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_io_num = I2C_MASTER_SCL_IO,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = I2C_MASTER_FREQ_HZ,
  };
  
  esp_err_t err = i2c_param_config(I2C_NUM_0, &conf);
  if (err != ESP_OK) 
  {
    return err;
  }

  return i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

void bsp_hw_init()
{
  bsp_uart_init();

  config_gpio(GPIO_RED_LED, GPIO_MODE_OUTPUT, false, false);
  config_gpio(GPIO_GREEN_LED, GPIO_MODE_OUTPUT, false, false);
  config_gpio(GPIO_STM32_RESET_N, GPIO_MODE_OUTPUT_OD, false, false);

  config_gpio(GPIO_STM32_TX_EN, GPIO_MODE_OUTPUT_OD, false, false);
  gpio_set_level(GPIO_STM32_TX_EN, 1);

  // Force a reset of the STM32 just in case
  gpio_set_level(GPIO_STM32_RESET_N, 0);
  usleep(2000);
  gpio_set_level(GPIO_STM32_RESET_N, 1);

  bsp_timer_init();
  
  if ( i2c_master_init() != ESP_OK )
    ESP_LOGE(TAG, "Unable to initialize I2C master 0");
}

void bsp_reboot()
{
  esp_restart(); 
}

int bsp_read_vbat()
{
  int sum = 0, cnt = 0;

  for ( int i = 0; i < 16; ++i )
  {
    uint8_t result[2] = {0};
    esp_err_t err = i2c_master_read_from_device(I2C_NUM_0, I2C_ADC_ADDRESS, result, sizeof(result), 100/portTICK_PERIOD_MS);
    if ( err != ESP_OK )
    {
      ESP_LOGE(TAG, "Error reading from ADC: %s", esp_err_to_name(err));
      return -1;
    }
    int mV = (result [0] << 8 | result[1]) * 3284 / 4095;
    //ESP_LOGI(TAG, "ADC reading: %.2x %.2x", result[0], result[1]);
    sum += mV;
    ++cnt;
  }

  int v = sum/cnt * 2; // Voltage is divided by two on the board

  //return mV * 2;
  return v;
}

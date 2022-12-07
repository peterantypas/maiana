#include "nmea_gateway.h"
#include "../bsp/bsp.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "types.h"

ESP_EVENT_DEFINE_BASE(NMEA_EVENT);

static TaskHandle_t __task_handle;
static StaticQueue_t __queue;
static QueueHandle_t __queue_handle;

#define QUEUE_LENGTH  20
static serial_message_t __queue_data[QUEUE_LENGTH];
static serial_message_t __buff = {0};
static int __pos = 0;

void uart_rx_cb(char c)
{
  __buff.text[__pos++] = c;
  if ( __pos == sizeof __buff.text )
    __pos = 0;

  if ( c == '\n' )
    {
      __buff.text[__pos] = 0;
      xQueueSend(__queue_handle, &__buff, portMAX_DELAY);
      __pos = 0;
    }
}

void nmea_input_task(void *params)
{
  serial_message_t msg;
  while (true)
  {
    if ( xQueueReceive(__queue_handle, &msg, portMAX_DELAY) == pdTRUE )
    {
      if ( msg.text[0] == '!' || msg.text[0] == '$' )
      {
        // This is a NMEA sentence
        printf(msg.text);
      }      
      else
      {
        // It's something else
      }
    }
  }
}

void nmea_gateway_start()
{
  bsp_set_uart_rx_cb(uart_rx_cb);

  __queue_handle = xQueueCreateStatic(QUEUE_LENGTH, sizeof(serial_message_t), (uint8_t*)__queue_data, &__queue);
  xTaskCreate(nmea_input_task, "nmea", 2048, NULL, 4, &__task_handle);
}

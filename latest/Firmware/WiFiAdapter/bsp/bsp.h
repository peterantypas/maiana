#ifndef __BSP_H__
#define __BSP_H__

#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(BSP_EVENT);

#define BSP_TX_BTN_EVENT    1

typedef void (uart_rx_cb_t)(char *in);

void bsp_hw_init();
void bsp_reboot();
void bsp_set_uart_rx_cb(uart_rx_cb_t *cb);

#endif

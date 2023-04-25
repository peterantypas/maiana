#ifndef __BSP_H__
#define __BSP_H__

#include "esp_event.h"


typedef void (uart_rx_cb_t)(char c);
typedef void (button_isr_cb_t)();
typedef void (timer_cb_t)();

void bsp_hw_init();
void bsp_reboot();
void bsp_set_uart_rx_cb(uart_rx_cb_t *cb);
void bsp_set_button_isr_cb(button_isr_cb_t *cb);
void bsp_set_timer_cb(timer_cb_t *cb);
void bsp_uart_write(const char *txt);
int bsp_read_vbat();

#endif

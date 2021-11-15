/*
 * printf2.c
 *
 *  Created on: Mar 1, 2016
 *      Author: peter
 */

#include "stm32l4xx_hal.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "config.h"


void USART_putc(USART_TypeDef* USARTx, char c)
{
  while ( !(USARTx->ISR & USART_ISR_TXE) )
    ;

  USARTx->TDR = c;
}


void USART_puts(USART_TypeDef* USARTx, const char *s)
{
  for ( int i = 0; s[i] != 0; ++i )
    USART_putc(USARTx, s[i]);
}

char __buffer[128];

#if ENABLE_VAR_ARGS
void printf2(const char *format, ...)
{
  va_list list;
  va_start(list, format);
  vsnprintf(__buffer, sizeof __buffer, format, list);
  va_end(list);

  USART_puts(USART1, __buffer);
}
#else
void printf2(const char *s)
{
  USART_puts(USART1, s);
}
#endif


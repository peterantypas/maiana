/*
 * printf2.h
 *
 *  Created on: Mar 1, 2016
 *      Author: peter
 */

#ifndef PRINTF2_H_
#define PRINTF2_H_


void printf2_Init(int baudrate);

// printf2 automatically determines ISR vs thread mode and schedules or prints immediately depending
void printf2(const char *format, ...);

// printf2_now outputs to UART immediately, disregarding ISR vs thread mode
void printf2_now(const char *format, ...);

#endif /* PRINTF2_H_ */

/*
 * printf2.h
 *
 *  Created on: Mar 1, 2016
 *      Author: peter
 */

#ifndef PRINTF2_H_
#define PRINTF2_H_

#include "config.h"

#if ENABLE_VAR_ARGS
void printf2(const char *format, ...);
#else
void printf2(const char *s);
#endif

#endif /* PRINTF2_H_ */

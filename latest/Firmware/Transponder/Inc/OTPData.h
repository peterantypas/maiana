/*
 * OTPData.h
 *
 *  Created on: Apr 5, 2021
 *      Author: peter
 */

#ifndef INC_OTPDATA_H_
#define INC_OTPDATA_H_

#include <stdint.h>

#define OTP_MAGIC     0x913A6D0C
#define OTP_REV       0x00000002


#define MCU_UNKNOWN       0x00
#define MCU_STM32L412     0x01
#define MCU_STM32L422     0x02
#define MCU_STM32L431     0x03
#define MCU_STM32L432     0x04

static const char *__mcuNames[] = {
    "unknown",
    "STM32L412",
    "STM32L422",
    "STM32L431",
    "STM32L432"
};


// This structure must be double-word aligned

typedef struct
{
  uint32_t  magic;
  uint32_t  rev;
  char      serialnum[32];
  char      hwrev[16];

  uint8_t   mcuType;

  uint8_t   reserved[7];
} OTPData;


#endif /* INC_OTPDATA_H_ */

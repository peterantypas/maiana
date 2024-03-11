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



// This structure must be double-word aligned

typedef struct
{
  uint32_t  magic;
  uint32_t  rev;
  char      serialnum[32];
  char      hwrev[16];
  uint8_t   reserved[8];
} OTPData;


#endif /* INC_OTPDATA_H_ */

/*
 * ConfigFlags.h
 *
 *  Created on: Oct 11, 2021
 *      Author: peter
 */

#ifndef INC_CONFIGFLAGS_H_
#define INC_CONFIGFLAGS_H_

typedef struct
{
  uint32_t magic;
  uint32_t reserved;
  uint32_t flags[5];
} ConfigFlags;

typedef union
{
  ConfigFlags config;
  uint64_t dw[4];
} ConfigPage;



#endif /* INC_CONFIGFLAGS_H_ */

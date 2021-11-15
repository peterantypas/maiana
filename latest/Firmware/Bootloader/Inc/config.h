/*
 * config.h
 *
 *  Created on: Aug 3, 2018
 *      Author: peter
 */

#ifndef CONFIG_H_
#define CONFIG_H_


// The bootloader occupies up to 16K at 0x08000000
#define METADATA_ADDRESS        0x08004000
#define APPLICATION_ADDRESS     0x08004800
#define ISR_VECTOR_OFFSET       0x00004800

#define BOOTMODE_ADDRESS        0x20009C00
#define DFU_FLAG_MAGIC          0xa191feed

#define ENABLE_VAR_ARGS         0


#endif /* CONFIG_H_ */

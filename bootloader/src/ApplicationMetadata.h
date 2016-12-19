/*
 * ApplicationMetadata.h
 *
 *  Created on: Jun 8, 2016
 *      Author: peter
 */

#ifndef APPLICATIONMETADATA_H_
#define APPLICATIONMETADATA_H_
#include <stm32f30x.h>



#define METADATA_ADDRESS        0x08002000
#define APPLICATION_ADDRESS     0x08002800
#define ISR_VECTOR_OFFSET       0x00004000

#define METADATA_MAGIC          0x0badf00d
#define FLASH_PAGE_SIZE         1024

typedef struct {
    uint32_t magic;
    char revision[8];
    uint32_t size;
    uint8_t md5[16];
}
ApplicationMetadata;


#endif /* APPLICATIONMETADATA_H_ */

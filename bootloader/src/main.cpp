//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include <stm32f30x.h>
#include "md5.h"
#include <cstring>
#include "ApplicationMetadata.h"
#include "DataTerminal.hpp"
#include "Timer.h"


// ----------------------------------------------------------------------------
//
// Standalone STM32F3 empty sample (trace via DEBUG).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

typedef  void (*pFunction)(void);

pFunction JumpToApplication;
uint32_t JumpAddress;

void jump();
void enterUARTMode();
bool verifyApplicationChecksum(const ApplicationMetadata &metadata);

Timer __timer;

void erase()
{
    FLASH_Unlock();
    FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
    FLASH_ErasePage(METADATA_ADDRESS);
    FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
    FLASH_Lock();
    FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
}

int
main(int argc, char* argv[])
{
    //erase();
  /*
   *  - Read page at METADATA_ADDRESS, look for software metadata
   *  - If no metadata is present, enter UART interactive mode and wait for software upload
   *  - If metadata is present, jump to software start address (APPLICATION_ADDRESS)
   */
#if 1
   ApplicationMetadata *metadata = (ApplicationMetadata*)METADATA_ADDRESS;
   if ( metadata == NULL || metadata->magic != METADATA_MAGIC ) {
       trace_printf("There is no software installed. Entering UART mode\n");
       enterUARTMode();

   }
#if 0
   else {

       if ( !verifyApplicationChecksum(*metadata) ) {
           trace_printf("The software is corrupt. Entering UART mode\n");
           enterUARTMode();
       }
   }
#endif

   trace_printf("Found software rev. %s\n", metadata->revision);
#endif

   pFunction start = (pFunction)(*(__IO uint32_t*) (APPLICATION_ADDRESS + 4));
   trace_printf("Application address: 0x%.8x\n", APPLICATION_ADDRESS);
   trace_printf("Jump address: 0x%.8x\n", start);

   __timer.start();

   Timer::sleep(2000);

   jump();
   //while(1);
}

void jump()
{
   /// Initialize user application's Stack Pointer
   __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);

   /// Reset interrupt vector to application
   NVIC_SetVectorTable(NVIC_VectTab_FLASH, ISR_VECTOR_OFFSET);

   // Start the application
   pFunction start = (pFunction)(*(__IO uint32_t*) (APPLICATION_ADDRESS + 4));
   start();
   //while(1);
}

void enterUARTMode()
{
    DataTerminal::instance().init();

    // Infinite loop
    while (1) {
        // Add your code here.
    }

}

bool verifyApplicationChecksum(const ApplicationMetadata &metadata)
{
#ifdef DEBUG
    return true;
#endif
    uint8_t hash[16];
    uint8_t *p = (uint8_t*)APPLICATION_ADDRESS;
    MD5_CTX ctx;
    MD5Init(&ctx);
    for ( uint32_t i = 0; i < metadata.size; ++i, ++p )
        MD5Update(&ctx, p, 1);

    MD5Final(hash, &ctx);

    return memcmp(hash, metadata.md5, sizeof hash) == 0;
}


#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------

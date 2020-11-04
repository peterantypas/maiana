/*
 * mem.c
 *
 *  Created on: Oct 8, 2020
 *      Author: peter
 */
#include "config.h"

#ifdef RTOS
#include "FreeRTOS.h"

void * operator new( size_t size )
{
  return pvPortMalloc( size );
}

void * operator new[]( size_t size )
{
  return pvPortMalloc(size);
}

void operator delete( void * ptr )
{
  vPortFree ( ptr );
}

void operator delete[]( void * ptr )
{
  vPortFree ( ptr );
}
#endif

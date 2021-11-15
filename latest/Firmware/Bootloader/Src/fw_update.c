/*
 * fw_update.c
 *
 *  Created on: Aug 3, 2018
 *      Author: peter
 */
#include "fw_update.h"
#include <string.h>
#include "printf2.h"
#include "config.h"
#include "stm32l4xx_hal.h"
#include <stdbool.h>
#include <stdlib.h>
#include "crc32.h"



#define MAX_BUFF                      128
#define GREETING                      "MAIANA bootloader ready\r\n"

extern void USART_putc(USART_TypeDef* USARTx, char c);
extern UART_HandleTypeDef huart1;

char __buff[MAX_BUFF];
uint8_t __p = 0;
uint8_t __page[FLASH_PAGE_SIZE];

void dfu_send(uint8_t b)
{
  USART_putc(USART1, b);
}

void dfu_init()
{
  __p = 0;
  memset(&firmwareUpdate, 0, sizeof firmwareUpdate);
  firmwareUpdate.state = WAITING;
  printf2(GREETING);

  // Turn on all LEDs
}

bool dfu_erase_flash();
bool dfu_flush_page();
uint32_t dfu_checksum_image();
bool dfu_write_metadata();
void dfu_tick();

int count_spaces(char *s)
{
  int r = 0;
  while ( *s )
    {
      if ( *s++ == ' ')
        ++r;
    }

  return r;
}

void dfu_process_load_cmd()
{
  int spaces = count_spaces(__buff);
  if ( spaces == 2 )
    {
      // OK, now we can parse it
      char *p1 = strchr(__buff, ' ');
      char *p2 = strchr(p1+1, ' ');
      uint32_t size = strtoul(p1, NULL, 10);
      uint32_t crc32 = strtoul(p2, NULL, 16);
      firmwareUpdate.image.size = size;
      firmwareUpdate.image.crc32 = crc32;

      if ( !dfu_erase_flash() )
        {
          // TODO: We failed. What now???
        }


      firmwareUpdate.state = TRANSFERRING;

      // Fill the buffer with all bits set to 1, so a partial write will leave flash bits untouched.
      memset(__page, 0xff, sizeof __page);
      firmwareUpdate.pos = 0;
      printf2("READY\r\n");
    }
}

void dfu_process_byte(uint8_t b)
{
  switch(firmwareUpdate.state)
  {
    case WAITING:
      if  ( b == '\n' )
        {
          __buff[__p++] = 0;

          // What command did we get?
          if ( strstr(__buff, "load ") )
            {
              dfu_process_load_cmd();
            }
          else if ( strlen(__buff) == 0 )
            {
              printf2(GREETING);
            }
          __p = 0;
        }
      else if ( b == '\r' )
        {
          __buff[__p++] = 0;
        }
      else
        {
          __buff[__p++] = b;
          if ( __p >= MAX_BUFF-1 )
            __p = 0;
        }
      break;
    case TRANSFERRING:
      {
        __page[firmwareUpdate.pos++] = b;
        ++firmwareUpdate.bytes;

        if ( firmwareUpdate.bytes % FLASH_PAGE_SIZE == 0 )
          {
            firmwareUpdate.pos = 0;
            ++firmwareUpdate.pages;

            if ( dfu_flush_page() )
              {
                printf2("OK\r\n");
              }
            else
              {
                printf2("ERROR: Flash failure\r\n");
              }
          }

        if ( firmwareUpdate.bytes == firmwareUpdate.image.size )
          {
            // Flash the remaining bytes (if any)
            if ( firmwareUpdate.bytes % FLASH_PAGE_SIZE )
              {
                ++firmwareUpdate.pages;
                if ( dfu_flush_page() )
                  {
                    //printf2("OK Done\r\n");
                  }
                else
                  {
                    printf2("ERROR: Flash failure\r\n");
                    dfu_init();
                  }
              }

            uint32_t crc = dfu_checksum_image();
            if ( crc == firmwareUpdate.image.crc32 )
              {
                printf2("OK, booting\r\n");
                HAL_Delay(100);
                dfu_write_metadata();
                NVIC_SystemReset();
              }
            else
              {
                printf2("ERROR: CRC mismatch\r\n");
                dfu_init();
              }
          }
        break;
      }
  }
}



bool dfu_erase_flash()
{
  // How many pages do we need to erase?
  uint8_t numPages = firmwareUpdate.image.size / FLASH_PAGE_SIZE;
  if ( firmwareUpdate.image.size % FLASH_PAGE_SIZE > 0 )
    ++numPages;

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
  if ( HAL_FLASH_Unlock() != HAL_OK )
    return false;

  FLASH_EraseInitTypeDef erase;
  erase.TypeErase = FLASH_TYPEERASE_PAGES;
  erase.Banks     = FLASH_BANK_1;
  erase.Page      = ISR_VECTOR_OFFSET / FLASH_PAGE_SIZE;
  erase.NbPages   = numPages;

  uint32_t errPage;
  HAL_StatusTypeDef status = HAL_OK;
  status = HAL_FLASHEx_Erase(&erase, &errPage);

  HAL_FLASH_Lock();

  return status == HAL_OK;
}

bool dfu_flush_page()
{
  uint32_t page = firmwareUpdate.pages-1;
  uint32_t pageAddress = APPLICATION_ADDRESS + page * FLASH_PAGE_SIZE;

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
  HAL_FLASH_Unlock();
  HAL_StatusTypeDef status = HAL_OK;
  for ( uint32_t dw = 0; dw < FLASH_PAGE_SIZE; dw += 8 )
    {
      uint64_t d = *(uint64_t*)&__page[dw];
      status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, pageAddress + dw, d);
      if ( status != HAL_OK )
        break;
    }
  HAL_FLASH_Lock();
  memset(__page, 0xff, sizeof __page);

  return status == HAL_OK;
}

uint32_t dfu_checksum_image()
{
  return crc32((void*)APPLICATION_ADDRESS, firmwareUpdate.image.size);
}


bool dfu_write_metadata()
{
  Metadata m;
  m.magic = 0xabadbabe;
  m.size = firmwareUpdate.image.size;
  m.crc32 = firmwareUpdate.image.crc32;
  m.flags = 0;
  uint32_t pageAddress = METADATA_ADDRESS;
  uint32_t page = (METADATA_ADDRESS - FLASH_BASE) / FLASH_PAGE_SIZE;

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
  HAL_FLASH_Unlock();

  FLASH_EraseInitTypeDef erase;
  erase.TypeErase = FLASH_TYPEERASE_PAGES;
  erase.Banks     = FLASH_BANK_1;
  erase.Page      = page;
  erase.NbPages   = 1;

  uint32_t errPage;
  HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &errPage);
  if ( status != HAL_OK )
    {
      HAL_FLASH_Lock();
      return false;
    }

  uint64_t *dw = (uint64_t*)&m;

  HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, pageAddress, *dw++);
  status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, pageAddress+8, *dw);
  if ( status != HAL_OK )
    {
      HAL_FLASH_Lock();
      return false;
    }

  HAL_FLASH_Lock();
  return true;
}

void dfu_tick()
{
  if ( firmwareUpdate.state == TRANSFERRING )
    {
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
    }
}


void USART1_IRQHandler(void)
{
  if ( __HAL_UART_GET_IT(&huart1, UART_IT_RXNE) )
    {
      __HAL_UART_CLEAR_IT(&huart1, UART_IT_RXNE);
      uint8_t c = USART1->RDR;
      dfu_process_byte(c);
    }
}





/*
  Copyright (c) 2016-2020 Peter Antypas

  This file is part of the MAIANA™ transponder firmware.

  The firmware is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>
*/

#include "stm32l4xx_hal.h"
#include "RadioManager.hpp"
#include "RXPacketProcessor.hpp"
#include "DataTerminal.hpp"
#include "TXScheduler.hpp"
#include "GPS.hpp"
#include "SystickTimer.hpp"
#include "CommandProcessor.hpp"
#include "bsp.hpp"
#include "printf_serial.h"
#include "FreeRTOS.h"
#include "task.h"

void fireTestPacket()
{
  VHFChannel channel = CH_87;

  if ( rand() % 2 == 0 )
    channel = CH_88;

  TXPacket *p = TXPacketPool::instance().newTXPacket(channel);
  if ( !p ) {
      //DBG("Ooops! Out of TX packets :(\r\n");
      return;
  }

  /**
   * Define a dummy packet of 9600 random bits, so it will take 1 second to transmit it.
   * This is long enough for most spectrum analyzers to capture details using "max hold",
   * even at very low resolution bandwidths. Great way to measure power and look for
   * spurious emissions as well as harmonics.
   */
  p->configureForTesting(channel, 9600);

  RadioManager::instance().sendTestPacketNow(p);
}

void determineCauseOfReset()
{
  std::string cause;
  if ( __HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) )
    cause += "Watchdog, ";
  if ( __HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) )
    cause += "Software, ";
  if ( __HAL_RCC_GET_FLAG(RCC_FLAG_BORRST) )
    cause += "Brownout";

  //DBG("Cause of reset: %s\r\n", cause.c_str());
  __HAL_RCC_CLEAR_RESET_FLAGS();
}


void mainTask(void *params)
{
  //uint32_t counter = 0;

  //bsp_start_wdt();

  // We're getting a very high rate of interrupts, so there's no need to dispatch events every time
  while (1)
    {
      //__WFI();
      vTaskDelay(10);
      //++counter;
      //if ( counter % 100 == 0 )
        //{
          //counter = 1;
          //bsp_refresh_wdt();
      EventQueue::instance().dispatch();
        //}
    }
}

class Ticker : EventConsumer
{
public:
  Ticker()
  {
    EventQueue::instance().addObserver(this, ONE_SEC_TIMER_EVENT);
  }

  void processEvent(const Event &e)
  {
    ++__t;
    printf_serial_now("Tick %d\r\n", __t);
  }
private:
  int __t = 0;
};

extern "C" {
__attribute__((used)) void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName )
{
  //printf_serial_now("WHOOPS!\r\n");
  asm("BKPT 0");
}

__attribute__((used)) void vApplicationTickHook()
{
  asm("BKPT 0");
}
}



int main(void)
{
  //*(uint8_t *)0xe000ed08 |= 2;
  bsp_hw_init();
  EventQueue::instance().init();
  SystickTimer::instance();
  Ticker t;
  //Configuration::instance().init();
  //CommandProcessor::instance().init();
  //DataTerminal::instance().init();
  //RXPacketProcessor packetProcessor;

#if not defined CALIBRATION_MODE && not defined TX_TEST_MODE
  //GPS::instance().init();
  //GPS::instance().enable();
#endif


#ifdef ENABLE_TX
  //TXPacketPool::instance().init();
  //TXScheduler::instance().init();
#endif

  //RadioManager::instance().init();
  //RadioManager::instance().start();

  TaskHandle_t xHandle;
  if ( xTaskCreate(mainTask, "main", 256u, NULL, tskIDLE_PRIORITY+1, &xHandle) != pdPASS )
    {
      asm("BKPT 0");
    }

  xPortStartScheduler();
}

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
#include "timers.h"

void jump_to_bootloader()
{
  typedef void (*pFunction)(void);
  pFunction systemBootloader;

  /**
   * System bootloader for L412 and L432 series resides at 0x1fff0000,
   * so the first 4 bytes contain the stack pointer and the next 4 contain the
   * program counter
   */
  systemBootloader = (pFunction) (*((uint32_t *)(0x1fff0004)));
  uint32_t *pp = (uint32_t*)0x1fff0000;
  uint32_t msp = *pp;
  __set_MSP(msp);

  // That's it, jump!
  systemBootloader();
}

TimerHandle_t timerHandle1, timerHandle2;
StaticTimer_t timer1, timer2;

extern "C"{
  void on1sec(TimerHandle_t handle)
  {
    Event e(ONE_SEC_TIMER_EVENT);
    EventQueue::instance().push(e);
  }

  void on1min(TimerHandle_t handle)
  {
    Event e(ONE_MIN_TIMER_EVENT);
    EventQueue::instance().push(e);
  }
}

void mainTask(void *params)
{
  EventQueue::instance().init();
  Configuration::instance().init();
  CommandProcessor::instance().init();
  DataTerminal::instance().init();

  RXPacketProcessor packetProcessor;

#if not defined CALIBRATION_MODE && not defined TX_TEST_MODE
  GPS::instance().init();
  GPS::instance().enable();
#endif


#ifdef ENABLE_TX
  TXPacketPool::instance().init();
  TXScheduler::instance().init();
#endif

  RadioManager::instance().init();
  RadioManager::instance().start();

  timerHandle1 = xTimerCreateStatic("1sec", 1000, pdTRUE, NULL, on1sec, &timer1);
  xTimerStart(timerHandle1, 10);

  timerHandle2 = xTimerCreateStatic("1min", 60000, pdTRUE, NULL, on1min, &timer2);
  xTimerStart(timerHandle2, 10);

  bsp_start_wdt();
  while (1)
    {
      EventQueue::instance().dispatch();
      vTaskDelay(50);
      bsp_refresh_wdt();
    }
}


int main(void)
{
#if 1
  if ( *(uint32_t*)DFU_FLAG_ADDRESS == DFU_FLAG_MAGIC )
    {
      *(uint32_t*)DFU_FLAG_ADDRESS = 0;
      jump_to_bootloader();
    }
#endif

  // This is for debugging imprecise bus faults
  //*(uint8_t *)0xe000ed08 |= 2;
  bsp_hw_init();

  TaskHandle_t xHandle;
  if ( xTaskCreate(mainTask, "main", 2048u, NULL, tskIDLE_PRIORITY+4, &xHandle) != pdPASS )
    {
      asm("BKPT 0");
    }

  vTaskStartScheduler();
  asm("BKPT 0");
  return 1;
}

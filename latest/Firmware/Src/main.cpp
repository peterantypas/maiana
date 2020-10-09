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

  timerHandle2 = xTimerCreateStatic("1min", 1000, pdTRUE, NULL, on1min, &timer2);
  xTimerStart(timerHandle2, 10);

  while (1)
    {
      EventQueue::instance().dispatch();
    }
}

extern "C" {
__attribute__((used)) void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName )
{
  //printf_serial_now("WHOOPS!\r\n");
  asm("BKPT 0");
}

}

int main(void)
{
  *(uint8_t *)0xe000ed08 |= 2;
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

//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include "printf2.h"
#include "GPS.hpp"
#include "RadioManager.hpp"
#include "EventQueue.hpp"
#include "LEDManager.hpp"
#include "TXPacket.hpp"
#include "RXPacketProcessor.hpp"
#include "TXScheduler.hpp"
#include "Utils.hpp"
#include "system_stm32f30x.h"
#include "core_cm4.h"
#include "stm32f30x.h"
#include <diag/Trace.h>
#include "DataTerminal.hpp"
#include "TXScheduler.hpp"
#include "DebugPrinter.hpp"
#include "EEPROM.hpp"

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



/*
 * DO NOT FORGET to modify this function when changing GPIO pins elsewhere in code.
 */
void configureUnusedPins();


int
main(int argc, char* argv[])
{
    // At this stage the system clock should have already been configured
    // at high speed.
    printf2_Init(230400);

#ifdef DEBUG
    // Disable buffered memory writes to debug imprecise bus faults
    SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;
    trace_printf("ACTLR: %.8x\n", SCnSCB->ACTLR);
#endif


    EEPROM::instance().init();
    /*
    struct StationData __d;
    __d.mmsi = 987654321;
    __d.len = 0;
    __d.beam = 0;
    strcpy(__d.callsign, "0N0000");
    strcpy(__d.name, "TEST STATION 01");
    EEPROM::instance().writeStationData(__d);
    */

    EventQueue::instance().init();
    EventPool::instance().init();
    TXPacketPool::instance().init();
    RXPacketPool::instance().init();

    LEDManager::instance().clear();
    TXScheduler txScheduler;

    DebugPrinter __db;

#ifdef ENABLE_TERMINAL
    DataTerminal::instance().init();
#endif


#if defined CALIBRATION_MODE
    RadioManager::instance().init();
    RadioManager::instance().transmitCW(CH_86);
#elif defined TX_TEST_MODE
    Radio::instance().init();
    GPS::instance().init();
    Radio::instance().start();
    Radio::instance().startReceiving(4);
    txScheduler.startTXTesting();
#else
    RXPacketProcessor packetProcessor;

    RadioManager::instance().init();
    GPS::instance().init();
    RadioManager::instance().start();
#endif


    configureUnusedPins();

    // Configure the watchdog timer
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_32);
    IWDG_SetReload(4095); // Max 12-bit value

    IWDG_Enable();

    // Dispatch events issued by ISRs and keep the watchdog happy
    printf2("Starting main event loop\r\n");
    while (true) {
        EventQueue::instance().dispatch();
        IWDG_ReloadCounter();
    }

    // If execution jumps here, something is seriously fucked up!!!
    assert(false);
}



void configureUnusedPins()
{
    // ST Micro recommends configuring all unused GPIOs as low outputs
    GPIO_InitTypeDef gpio;

    // Port A pins
    uint32_t pins = GPIO_Pin_15 | GPIO_Pin_11 | GPIO_Pin_12;
    gpio.GPIO_Pin = pins;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Speed = GPIO_Speed_Level_1;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &gpio);
    GPIO_ResetBits(GPIOA, pins);

    // Port B pins
    pins = GPIO_Pin_0 | GPIO_Pin_5;
    gpio.GPIO_Pin = pins;
    GPIO_Init(GPIOB, &gpio);
    GPIO_ResetBits(GPIOB, pins);

    // Port C pins
    pins = GPIO_Pin_14 | GPIO_Pin_15;
    gpio.GPIO_Pin = pins;
    GPIO_Init(GPIOC, &gpio);
    GPIO_ResetBits(GPIOC, pins);
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------

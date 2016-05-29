/*
 * DebugPrinter.cpp
 *
 *  Created on: Apr 6, 2016
 *      Author: peter
 */

#include "DebugPrinter.hpp"
#include "EventQueue.hpp"
#include "printf2.h"

DebugPrinter::DebugPrinter()
{
    EventQueue::instance().addObserver(this, DEBUG_EVENT);
}

DebugPrinter::~DebugPrinter()
{
}

void DebugPrinter::processEvent(Event *e)
{
    DebugEvent *event = static_cast<DebugEvent*>(e);
    printf2(event->mBuffer);
}


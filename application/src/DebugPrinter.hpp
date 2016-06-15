/*
 * DebugPrinter.hpp
 *
 *  Created on: Apr 6, 2016
 *      Author: peter
 */

#ifndef DEBUGPRINTER_HPP_
#define DEBUGPRINTER_HPP_

#include "Events.hpp"

class DebugPrinter : public EventConsumer
{
public:
    DebugPrinter();
    virtual ~DebugPrinter();

    void processEvent(const Event &e);
};

#endif /* DEBUGPRINTER_HPP_ */

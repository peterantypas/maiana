/*
 * Terminal.hpp
 *
 *  Created on: Apr 19, 2016
 *      Author: peter
 */

#ifndef DATATERMINAL_HPP_
#define DATATERMINAL_HPP_

#include "globals.h"
#include "EventQueue.hpp"
#ifdef ENABLE_TERMINAL


class DataTerminal : public EventConsumer
{
public:
    static DataTerminal &instance();

    void init();

    void processEvent(Event *e);

    void write(const char* line, bool interactive = false);
    void processCharacter(char c);
private:
    DataTerminal();

    typedef enum {
        MAIN_SCREEN = 0,
        PROGRAMMING_SCREEN = 1
    }
    MenuScreen;

    void showScreen(MenuScreen s);

    void clearScreen();
    void _write(const char* s);
private:
    //uint32_t mTimeSlot;
    uint8_t mEscapes;
    MenuScreen mCurrentScreen;
    bool mInteractive;
};
#endif



#endif /* DATATERMINAL_HPP_ */

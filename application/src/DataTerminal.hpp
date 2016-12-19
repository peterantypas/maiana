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
#include <vector>
#include <string>

using namespace std;

#define ENABLE_TERMINAL

#ifdef ENABLE_TERMINAL


class DataTerminal : public EventConsumer
{
public:
    static DataTerminal &instance();

    void init();

    void processEvent(const Event &e);
#ifdef MULTIPLEXED_OUTPUT
    void write(const char* cls, const char* line);
#else
    void write(const char* line);
#endif
    void processCharacter(char c);
private:
    DataTerminal();
    void processCommand();

    void _write(const char* s);
private:
    char mCmdBuffer[64];
    size_t mCmdBuffPos;
    vector<string> mCmdTokens;
};
#endif



#endif /* DATATERMINAL_HPP_ */

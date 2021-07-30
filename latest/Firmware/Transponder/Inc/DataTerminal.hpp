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

#ifndef DATATERMINAL_HPP_
#define DATATERMINAL_HPP_

#include "EventQueue.hpp"
#include <vector>
#include <string>
#include "config.h"

using namespace std;


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
private:
  DataTerminal();
  void processCommand(const char*);

  void _write(const char* s);
private:
  char mCmdBuffer[64];
  size_t mCmdBuffPos;
  vector<string> mCmdTokens;
};
#endif




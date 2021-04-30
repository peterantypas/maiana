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


#ifndef SRC_COMMANDPROCESSOR_HPP_
#define SRC_COMMANDPROCESSOR_HPP_

#include "Events.hpp"


class CommandProcessor : public EventConsumer
{
public:
  static CommandProcessor &instance();

  void init();

  void processEvent(const Event &e);

private:
  void processCommand(const char *);
  void jumpToBootloader();
  void dumpOTPData();
  void writeOTPData(const std::string &cmd);
  void enterCLIMode();
  CommandProcessor();
};



#endif /* SRC_COMMANDPROCESSOR_HPP_ */

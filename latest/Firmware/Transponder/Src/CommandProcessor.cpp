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


#include "CommandProcessor.hpp"
#include "EventQueue.hpp"
#include "Utils.hpp"
#include "TXScheduler.hpp"
#include "config.h"
#include "StationData.h"
#include "TXScheduler.hpp"
#include "bsp.hpp"
#include "GPS.hpp"
#include "RadioManager.hpp"
#include <stdlib.h>

CommandProcessor &CommandProcessor::instance()
{
  static CommandProcessor __instance;
  return __instance;
}

void CommandProcessor::init()
{

}

CommandProcessor::CommandProcessor()
{
  EventQueue::instance().addObserver(this, COMMAND_EVENT);
}

void CommandProcessor::processEvent(const Event &e)
{
  switch(e.type)
  {
  case COMMAND_EVENT:
    processCommand(e.command.buffer);
    break;
  default:
    break;
  }
}

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
   * Define a dummy packet of 9600 random bits, so it will take 1 second to transmit.
   * This is long enough for most spectrum analyzers to capture details using "max hold",
   * even at very low resolution bandwidths. Great way to measure power and look for
   * spurious emissions as well as harmonics.
   */
  p->configureForTesting(channel, 9600);

  RadioManager::instance().sendTestPacketNow(p);
}

void CommandProcessor::processCommand(const char *buff)
{
  string s(buff);
  Utils::trim(s);

  if ( s.find("station ") == 0 )
    {
      /*
       * The station command format is:
       * station mmsi,name,callsign,type,len,beam,portoffset,bowoffset
       */

      StationData station;

      string params = s.substr(8);
      if (params.empty())
        return;

      vector<string> tokens;
      Utils::tokenize(params, ',', tokens);
      if ( tokens.size() < 8 )
        return;

      memset(&station, 0, sizeof station);
      station.mmsi = Utils::toInt(tokens[0]);
      strlcpy(station.name, tokens[1].c_str(), sizeof station.name);
      strlcpy(station.callsign, tokens[2].c_str(), sizeof station.callsign);
      int type = (VesselType)Utils::toInt(tokens[3]);
      if ( type == 30 || type == 34 || type == 36 || type == 37 )
        station.type = (VesselType)type;
      station.len = Utils::toInt(tokens[4]);
      station.beam = Utils::toInt(tokens[5]);
      station.portOffset = Utils::toInt(tokens[6]);
      station.bowOffset = Utils::toInt(tokens[7]);
      station.magic = STATION_DATA_MAGIC;

      Configuration::instance().writeStationData(station);
    }
  else if ( s.find("station?") == 0 )
    {
      Configuration::instance().reportStationData();
    }
  else if ( s.find("sys?") == 0 )
    {
      Configuration::instance().reportSystemData();
    }
  else if ( s.find("dfu") == 0 )
    {
      jumpToBootloader();
    }
  else if ( s.find("erase station") == 0 )
    {
      Configuration::instance().eraseStationData();
      Configuration::instance().reportStationData();
    }
  else if ( s.find("factory reset") == 0 )
    {
      Configuration::instance().factoryReset();
      Configuration::instance().reportStationData();
    }
  else if ( s.find("tx test") == 0 )
    {
      fireTestPacket();
    }
  else if ( s.find("tx on") == 0 )
    {
      Configuration::instance().enableTX();
      TXScheduler::instance().reportTXStatus();
    }
  else if ( s.find("tx off") == 0 )
    {
      Configuration::instance().disableTX();
      TXScheduler::instance().reportTXStatus();
    }
  else if ( s.find("tx?") == 0 )
    {
      TXScheduler::instance().reportTXStatus();
    }
  else if ( s.find("txcw a") == 0 )
    {
      RadioManager::instance().transmitCW(CH_87);
    }
  else if ( s.find("txcw b") == 0 )
    {
      RadioManager::instance().transmitCW(CH_88);
    }
  else if ( s.find("stoptx") == 0 )
    {
      RadioManager::instance().stopTX();
    }
  else if (s.find("reboot") == 0 )
    {
      bsp_reboot();
    }
  else if ( s.find("cli") == 0 )
    {
      enterCLIMode();
    }
  else if ( s.find("gps off") == 0 )
    {
      bsp_gnss_off();
    }
  else if ( s.find("gps on") == 0 )
    {
      bsp_gnss_on();
    }
  else if ( s.find("msg24") == 0 )
    {
      TXScheduler::instance().queueMessage24(CH_87);
    }
#if OTP_DATA
  else if ( s.find("otp?") == 0 )
    {
      dumpOTPData();
    }
  else if ( s.find("otp ") == 0 )
    {
      writeOTPData(s);
    }
#endif
  else if ( s.find("xotrim ") == 0 )
    {
      uint8_t value = Utils::toInt(s.substr(7));
      RadioManager::instance().setXOTrimValue(value);
      Configuration::instance().setXOTrimValue(value);
    }
  else if ( s.find("xotrim?") == 0 )
    {
      Configuration::instance().reportXOTrimValue();
    }
}

void CommandProcessor::enterCLIMode()
{
  *(uint32_t*)BOOTMODE_ADDRESS = CLI_FLAG_MAGIC;
  bsp_reboot();
}

void CommandProcessor::jumpToBootloader()
{
  bsp_enter_dfu();
}

#if OTP_DATA
void CommandProcessor::dumpOTPData()
{
  Configuration::instance().reportOTPData();
}

void CommandProcessor::writeOTPData(const std::string &s)
{
  string params = s.substr(4);
  if (params.empty())
    return;

  vector<string> tokens;
  Utils::tokenize(params, ' ', tokens);

  OTPData data;
  memset(&data, 0, sizeof data);

  data.magic  = OTP_MAGIC;
  data.rev    = OTP_REV;
  strlcpy(data.hwrev, tokens[0].c_str(), sizeof data.hwrev);

  if ( tokens.size() > 2 )
    strlcpy(data.serialnum, tokens[1].c_str(), sizeof data.serialnum);

  bool result = Configuration::instance().writeOTP(data);
  if ( result )
    dumpOTPData();
}
#endif


/*
 * CommandProcessor.hpp
 *
 *  Created on: Jun 24, 2016
 *      Author: peter
 */

#ifndef COMMANDPROCESSOR_HPP_
#define COMMANDPROCESSOR_HPP_

#include "EventQueue.hpp"

class CommandProcessor : public EventConsumer
{
public:
  static CommandProcessor &instance();

  void init();
  void processEvent(const Event &);
private:
  CommandProcessor();

  void returnMMSI();
  void returnName();
  void returnCallSign();
  void returnBeam();
  void returnLength();
  void returnMode();
  void returnVesselData();
  void returnStatus();
  void returnVersion();


  void setMMSI(uint32_t mmsi);
  void setName(const char* name);
  void setCallSign(const char *callsign);
  void setBeam(uint8_t beam);
  void setLength(uint8_t len);
  void setVesselData(const char *data);
  void setMode(const char *mode);

  void sendError(const char* err);
  void sendEmptyReply(bool success);
};

#endif /* COMMANDPROCESSOR_HPP_ */

/*
 * NMEA0183Consumer.hpp
 *
 *  Created on: Jun 30, 2021
 *      Author: peter
 */

#ifndef INC_NMEA0183CONSUMER_HPP_
#define INC_NMEA0183CONSUMER_HPP_


#include <EventQueue.hpp>
#include <NMEA2000.h>
#include <N2kTypes.h>
#include "NMEASentence.hpp"

#include <map>
#include <list>

using namespace std;

class NMEA0183Consumer : public EventConsumer
{
public:
  static NMEA0183Consumer &instance();
  void init(tNMEA2000 *n2k);
  void onRX(char c);
  void processEvent(const Event &e);
private:
  NMEA0183Consumer();
  void processAISSentence(const char *);
  void processGPSSentence(const char *);
  void processAISBody(tN2kAISTranceiverInfo aisInfo, const string &body, int bitPadding);
  int leapSecondsSince1970();
  uint16_t calcETADate(int month, int day);
  uint32_t calcETATime(int hour, int minute);
  tN2kAISTranceiverInfo transceiverInfo(const string &channel, const string &msgCode);
private:
  char mBuff[120];
  uint32_t mBuffPos = 0;
  tNMEA2000 *mN2K = nullptr;
  typedef map<int, list<NMEASentence> > NMEAFragments;
  NMEAFragments mNMEAFragments;
  time_t mUTC = 0;
  struct tm mTime = {0};
};


#endif /* INC_NMEA0183CONSUMER_HPP_ */

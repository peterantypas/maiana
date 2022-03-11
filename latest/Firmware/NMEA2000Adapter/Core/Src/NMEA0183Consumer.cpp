/*
 * NMEA0183Consumer.cpp
 *
 *  Created on: Jun 30, 2021
 *      Author: peter
 */

#include "NMEA0183Consumer.hpp"
#include "EventQueue.hpp"
#include <bsp/bsp.hpp>
#include <string.h>
#include "NMEASentence.hpp"
#include "Utils.hpp"
#include <list>
#include <decode_body.h>
#include <ais.h>
#include <N2kMessages.h>
#include <math.h>

using namespace std;
using namespace libais;


static const int MonthDays[] = {
    31, // Jan
    28, // Feb, but what about leap years?
    31, // Mar
    30, // Apr
    31, // May
    30, // Jun
    31, // Jul
    31, // Aug
    30, // Sep
    31, // Oct
    30, // Nov
    31, // Dec
};

NMEA0183Consumer::NMEA0183Consumer()
{

}

NMEA0183Consumer &NMEA0183Consumer::instance()
{
  static NMEA0183Consumer __instance;
  return __instance;
}

void onRxIRQ(char c)
{
  NMEA0183Consumer::instance().onRX(c);
}

void NMEA0183Consumer::init(tNMEA2000 *n2k)
{
  mN2K = n2k;
  bsp_set_uart_irq_cb(onRxIRQ);
  EventQueue::instance().addObserver(this, GPS_NMEA_SENTENCE | AIS_NMEA_SENTENCE);
}

void NMEA0183Consumer::onRX(char c)
{
  mBuff[mBuffPos++] = c;
  if (mBuffPos > 90)
    {
      // We screwed up!
      mBuffPos = 0;
      mBuff[mBuffPos] = 0;
    }
  else if ( c == '\r' )
    {
      // Do nothing
    }
  else if (c == '\n')
    {
      mBuff[mBuffPos] = 0;
      Event *e = nullptr;
      if ( strstr(mBuff, "$G") == mBuff )
        {
          e = EventPool::instance().newEvent(GPS_NMEA_SENTENCE);
        }
      else if ( strstr(mBuff, "!AI") == mBuff )
        {
          e = EventPool::instance().newEvent(AIS_NMEA_SENTENCE);
        }

      if ( e )
        {
          strlcpy(e->nmeaBuffer.sentence, mBuff, sizeof e->nmeaBuffer.sentence);
          EventQueue::instance().push(e);
        }
      mBuffPos = 0;
      mBuff[mBuffPos] = 0;
    }
}

void NMEA0183Consumer::processEvent(const Event &e)
{
  switch(e.type)
  {
    case GPS_NMEA_SENTENCE:
      processGPSSentence(e.nmeaBuffer.sentence);
      break;
    case AIS_NMEA_SENTENCE:
      processAISSentence(e.nmeaBuffer.sentence);
      break;
    default:
      break;
  }
}

void NMEA0183Consumer::processGPSSentence(const char *str)
{
  NMEASentence sentence(str);

  if ( mUTC && (sentence.code() == "GPGGA" || sentence.code() == "GNGGA") )
    {
      if (sentence.fields()[2].length() == 0 || sentence.fields()[4].length() == 0)
        return;

      double lat = Utils::latitudeFromNMEA (sentence.fields()[2], sentence.fields()[3]);
      double lng = Utils::longitudeFromNMEA (sentence.fields()[4], sentence.fields()[5]);


      tN2kMsg pos;
      SetN2kPGN129029(pos,
                      1,
                      (mUTC-leapSecondsSince1970()-1)/86400.0f,
                      fmod(mUTC-leapSecondsSince1970()-1, 86400.0f),
                      lat,
                      lng,
                      atof(sentence.fields()[9].c_str()),   // Altitude
                      N2kGNSSt_GPSGLONASS,                  // Hard-coded, since this is MAIANA
                      N2kGNSSm_DGNSS,                       // Also hard-coded, it's always going to be this
                      Utils::toInt(sentence.fields()[7]),   // Number of satellites
                      atof(sentence.fields()[8].c_str()),   // HDOP
                      0.01,                                 // PDOP
                      atof(sentence.fields()[11].c_str()),  // Geoid height here
                      0xff,                                 // We don't specify stations
                      N2kGNSSt_GPSGLONASS,
                      1,
                      1);

      if ( mN2K->SendMsg(pos) )
        {
          //printf("Sent PGN129029\r\n");
        }

    }

  if ( sentence.code() == "GNRMC" || sentence.code() == "GPRMC" )
    {
      const vector<string> &fields = sentence.fields ();

      if (fields.size () < 10)
        return;

      if (fields[1].length () < 6 || fields[9].length () < 6)
        return;

      double lat, lng, cog, sog;
      if (sentence.fields()[3].length() > 0 && sentence.fields()[5].length() > 0)
        {
          lat = Utils::latitudeFromNMEA (sentence.fields()[3], sentence.fields()[4]);
          lng = Utils::longitudeFromNMEA (sentence.fields()[5], sentence.fields()[6]);
          tN2kMsg pos;
          SetN2kLatLonRapid(pos, lat, lng);
          if ( mN2K->SendMsg(pos) )
            {
              //printf("Sent PGN129025\r\n");
            }

          sog = atof(sentence.fields()[7].c_str());
          cog = atof(sentence.fields()[8].c_str());

          const string &timestr = fields[1].substr (0, 6);
          const string &datestr = fields[9].substr (0, 6);
          mTime.tm_hour = Utils::toInt (timestr.substr (0, 2));
          mTime.tm_min = Utils::toInt (timestr.substr (2, 2));
          mTime.tm_sec = Utils::toInt (timestr.substr (4, 2));
          mTime.tm_mday = Utils::toInt (datestr.substr (0, 2));
          mTime.tm_mon = Utils::toInt (datestr.substr (2, 2)) - 1; // Month is 0-based
          mTime.tm_year = 100 + Utils::toInt (datestr.substr (4, 2)); // Year is 1900-based
          mUTC = mktime (&mTime);


          tN2kMsg cogsog;
          SetN2kCOGSOGRapid(cogsog,
                            1,
                            N2khr_true,
                            DegToRad(cog),
                            KnotsToms(sog));
          if ( mN2K->SendMsg(cogsog) )
            {
              //printf("Sent PGN129026\r\n");
            }
        }
    }
}

int NMEA0183Consumer::leapSecondsSince1970()
{
  // TODO: This needs to be updated going forward ...
  return 27;
}

tN2kAISTranceiverInfo NMEA0183Consumer::transceiverInfo(const string &channel, const string &msgCode)
{
  if ( msgCode == "AIVDM" )
    {
      return channel=="A" ? N2kaisti_Channel_A_VDL_reception : N2kaisti_Channel_B_VDL_reception;
    }
  else if ( msgCode == "AIVDO" )
    {
      return channel=="A" ? N2kaisti_Channel_A_VDL_transmission : N2kaisti_Channel_B_VDL_transmission;
    }

  return N2kaisti_Reserved;
}

void NMEA0183Consumer::processAISSentence(const char *str)
{
  //printf("processAISSentence()\r\n");
  NMEASentence s(str);

  const string &chStr = s.fields()[4];
  const string &code = s.code();

  tN2kAISTranceiverInfo aisInfo = transceiverInfo(chStr, code);

  int fragmentCount = Utils::toInt(s.fields()[1]);
  if ( fragmentCount == 1 )
    {
      // Just process this as a complete message
      list<NMEASentence> l;
      l.push_back(s);
      processAISBody(aisInfo, s.fields()[5], Utils::toInt(s.fields()[6]));
    }
  else
    {
      int fragmentNum = Utils::toInt(s.fields()[2]);
      int messageId = Utils::toInt(s.fields()[3]);

      // Accumulate the fragment
      NMEAFragments::iterator it = mNMEAFragments.find(messageId);
      if ( it == mNMEAFragments.end() )
        {
          list<NMEASentence> l;
          l.push_back(s);
          mNMEAFragments[messageId] = l;
        }
      else
        {
          it->second.push_back(s);
        }

      if ( fragmentNum == fragmentCount )
        {
          // This is a complete message now, process it
          string body = "";
          int padding = 0;
          for ( list<NMEASentence>::iterator sIt = it->second.begin(); sIt != it->second.end(); ++sIt )
            {
              body += sIt->fields()[5];
              padding = Utils::toInt(sIt->fields()[6]); // All but the last fragment will have zero bit padding
            }

          //printf("Body: %s\r\n", body.c_str());
          processAISBody(aisInfo, body, padding);
          mNMEAFragments.erase(it);
        }
    }
}

/**
 * TODO: This function will be off by a day on leap years.
 * Once GNSS data flows through this system regularly, then this can be fixed.
 */

uint16_t NMEA0183Consumer::calcETADate(int month, int day)
{
  // TODO: This must really return "today" to deal with unspecified ETA, otherwise it will be
  // interpreted as January 1st. I'm sure NMEA 2000 defines an "unspecified" value here
  // but it's not 0xffff :(
  if ( month == 0 || day == 0 )
    return N2kUInt16NA;

  uint16_t res = 0;
  for ( int i = 0; i < month-1; i++ )
    {
      res += MonthDays[i];
    }
  res += day-1;
  return res;
}

uint32_t NMEA0183Consumer::calcETATime(int hour, int minute)
{
  if ( hour >= 24 || minute >= 60 )
    return 0;

  // This is seconds since midnight
  return hour * 3600 + minute * 60;
}

void NMEA0183Consumer::processAISBody(tN2kAISTranceiverInfo aisInfo, const string &body, int bitPadding)
{
  // This will delete itself when it goes out of scope
  std::unique_ptr<AisMsg> msg = CreateAisMsg(body, bitPadding);
  if ( !msg.get() )
    {
      //printf("Failed to parse AIS body\r\n");
      return;
    }

  // printf("Parsed AIS body, msg type = %d\r\n", msg->message_id);

  switch(msg->message_id)
  {
    case 1:
    case 2:
    case 3:
      {
        // Position report, class A => PGN 129038
        Ais1_2_3 *m = static_cast<Ais1_2_3*>(msg.get());
        tN2kMsg n2kmsg;

        SetN2kAISClassAPosition(n2kmsg,
                                m->message_id,
                                (tN2kAISRepeat)m->repeat_indicator,
                                m->mmsi,
                                m->position.lat_deg,
                                m->position.lng_deg,
                                m->position_accuracy,
                                m->raim,
                                m->timestamp%60,
                                DegToRad(m->cog),
                                KnotsToms(m->sog),
                                (m->true_heading == 511 ? N2kDoubleNA : DegToRad(m->true_heading)),
                                (m->rot_over_range ? 3.125E-05 : m->rot),
                                (tN2kAISNavStatus)m->nav_status,
                                aisInfo);

        if ( mN2K->SendMsg(n2kmsg) )
          {
            //printf("Sent PGN129038, aisInfo=%d:\r\n", aisInfo);
          }
      }
      break;
    case 5:
      {
        // Static report, class A => PGN 129794
        Ais5 *m = static_cast<Ais5*>(msg.get());
        tN2kMsg n2kmsg;

        SetN2kAISClassAStatic(n2kmsg,
                              m->message_id,
                              (tN2kAISRepeat)m->repeat_indicator,
                              m->mmsi,
                              m->imo_num,
                              m->callsign.c_str(),
                              m->name.c_str(),
                              m->type_and_cargo, // Does this need some bit magic, or is it type + cargo ?
                              m->dim_a+m->dim_b,
                              m->dim_c+m->dim_d,
                              m->dim_d,
                              m->dim_a,
                              calcETADate(m->eta_month, m->eta_day),
                              calcETATime(m->eta_hour, m->eta_minute),
                              m->draught,
                              m->destination.c_str(),
                              (tN2kAISVersion)m->ais_version,
                              (tN2kGNSStype)m->fix_type,
                              (tN2kAISDTE)m->dte,
                              aisInfo);

        if ( mN2K->SendMsg(n2kmsg) )
          {
            //printf("Sent PGN129794:\r\n");
          }
        else
          {
            //printf("Failed to send PGN 129794\r\n");
          }
      }
      break;
    case 18:
      {
        // Position report, class B => PGN 12039
        Ais18 *m = static_cast<Ais18*>(msg.get());
        tN2kMsg n2kmsg;

        SetN2kAISClassBPosition(n2kmsg,
                                m->message_id,
                                (tN2kAISRepeat)m->repeat_indicator,
                                m->mmsi,
                                m->position.lat_deg,
                                m->position.lng_deg,
                                m->position_accuracy,
                                m->raim,
                                m->timestamp%60,
                                DegToRad(m->cog),
                                KnotsToms(m->sog),
                                (m->true_heading == 511 ? N2kDoubleNA : DegToRad(m->true_heading)),
                                (tN2kAISUnit)m->unit_flag,
                                m->display_flag,
                                m->dsc_flag,
                                m->band_flag,
                                m->m22_flag,
                                (tN2kAISMode)m->mode_flag,
                                m->commstate_flag,
                                aisInfo);
        if ( mN2K->SendMsg(n2kmsg) )
          {
            //printf("Sent PGN129039\r\n");
          }
        else
          {
            //printf("Failed to send PGN 129039\r\n");
          }
      }
      break;
    case 21:
      {
        // ATON report => PGN 129041
        Ais21 *m = static_cast<Ais21*>(msg.get());
        tN2kMsg n2kmsg;

        SetN2kPGN129041(n2kmsg,
                        m->message_id,
                        (tN2kAISRepeat)m->repeat_indicator,
                        m->mmsi,
                        m->position.lat_deg,
                        m->position.lng_deg,
                        m->position_accuracy,
                        m->raim,
                        m->timestamp,
                        (tN2kAtoNType)m->aton_type,
                        m->name.c_str(),
                        m->dim_a+m->dim_b,
                        m->dim_c+m->dim_d,
                        m->dim_d,
                        m->dim_a,
                        m->off_pos,
                        m->virtual_aton,
                        m->assigned_mode,
                        (tN2kGNSStype)m->fix_type,
                        aisInfo);

        if ( mN2K->SendMsg(n2kmsg) )
          {
            //printf("Sent PGN129041\r\n");
          }
        else
          {
            //printf("Failed to send PGN 129041\r\n");
          }
      }
      break;
    case 24:
      {
        // Static report, class B => PGN 129809 (part A) or PGN 129810 (part B)
        Ais24 *m = static_cast<Ais24*>(msg.get());
        tN2kMsg n2kmsg;

        if ( m->part_num == 0 )
          {
            SetN2kAISClassBStaticPartA(n2kmsg,
                                       m->message_id,
                                       (tN2kAISRepeat)m->repeat_indicator,
                                       m->mmsi,
                                       m->name.c_str(),
                                       aisInfo);
          }
        else
          {
            SetN2kAISClassBStaticPartB(n2kmsg,
                                       m->message_id,
                                       (tN2kAISRepeat)m->repeat_indicator,
                                       m->mmsi,
                                       m->type_and_cargo,
                                       m->vendor_id.c_str(),
                                       m->callsign.c_str(),
                                       m->dim_a + m->dim_b,
                                       m->dim_c+m->dim_d,
                                       m->dim_d,
                                       m->dim_a,
                                       0,
                                       aisInfo);


          }

        if ( mN2K->SendMsg(n2kmsg) )
          {
            if ( m->part_num == 0 )
              {
                //printf("Sent PGN129809\r\n");
              }
            else
              {
                //printf("Sent PGN129810\r\n");
              }
          }
        else
          {
            //printf("Failed to send PGN129809/810\r\n");
          }
      }
      break;
    default:
      break;
  }
}



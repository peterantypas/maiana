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


#include "GPS.hpp"
#include "DataTerminal.hpp"
#include "config.h"
#include "NMEASentence.hpp"
#include "Utils.hpp"
#include "EventQueue.hpp"
#include "bsp.hpp"
#include <stdio.h>
#include <stdlib.h>


void gnssInputCB(char c);
void gnss1PPSCB();
void gnssSOTDMACB();

GPS &
GPS::instance()
{
  static GPS __instance;
  return __instance;
}

GPS::GPS()
: mBuffPos(0), mUTC(0), mLat(0), mLng(0), mStarted(false), mSlotNumber(0), mDelegate(NULL), mCOG(511), mSpeed(0)
{
  memset(&mTime, 0, sizeof(mTime));
  mPeriod = (bsp_get_system_clock() / 37.5) - 1;
  EventQueue::instance().addObserver(this, GPS_NMEA_SENTENCE);
}

GPS::~GPS()
{

}

time_t GPS::UTC()
{
  return mUTC;
}

struct tm &GPS::time()
{
  return mTime;
}

uint32_t GPS::aisSlot()
{
  return mSlotNumber;
}

double GPS::lat()
{
  return mLat;
}

double GPS::lng()
{
  return mLng;
}

void GPS::setDelegate(GPSDelegate *delegate)
{
  mDelegate = delegate;
}

void GPS::init()
{
  bsp_set_gnss_input_callback(gnssInputCB);
  bsp_set_gnss_1pps_callback(gnss1PPSCB);
  bsp_set_gnss_sotdma_timer_callback(gnssSOTDMACB);
}

void GPS::enable()
{
  bsp_gnss_on();
}

void GPS::disable()
{
  bsp_gnss_off();
  bsp_gps_led_off();
}

bool GPS::isEnabled()
{
  return bsp_is_gnss_on();
}

void GPS::onRX(char c)
{
  // This code is called in interrupt mode, do as little as possible!

  mBuff[mBuffPos++] = c;
  if (mBuffPos > 90)
    {
      // We screwed up!
      mBuffPos = 0;
      mBuff[mBuffPos] = 0;
    }
  else if (c == '\n')
    {
      mBuff[mBuffPos] = 0;
      Event *e = EventPool::instance().newEvent(GPS_NMEA_SENTENCE);
      if ( e )
        {
          strlcpy(e->nmeaBuffer.sentence, mBuff, sizeof e->nmeaBuffer.sentence);
          EventQueue::instance ().push(e);
        }
      mBuffPos = 0;
      mBuff[mBuffPos] = 0;
    }
}

void GPS::onPPS()
{
  // If we don't have time yet, we can't use this
  if ( mUTC == 0 )
    return;

  ++mUTC;     // PPS := advance clock by one second!
  localtime_r (&mUTC, &mTime); // Now we know exactly what UTC second it is, with only microseconds of latency
  if (!mStarted)
    {
      // To keep things simple, we only start the AIS slot timer if we're on an even second (it has a 37.5 Hz frequency)
      mSlotNumber = (mTime.tm_sec % 60) * 2250; // We know what AIS slot number we're in
      if (!(mTime.tm_sec & 0x00000001))
        startTimer ();
    }
  else
    {
      // The timer is on, now let's re-calibrate ...
      if (mTime.tm_sec & 1)
        {
          // On odd seconds, we expect the timer value to be half its period. Just correct it.
          uint32_t nominalTimerValue = mPeriod / 2 - 1;
          //TIM2->CNT = nominalTimerValue;
          bsp_set_sotdma_timer_value(nominalTimerValue);
        }
      else
        {
          // On even seconds, things are a bit more tricky ...
          //uint32_t currentTimerValue = TIM2->CNT;
          uint32_t currentTimerValue = bsp_get_sotdma_timer_value();
          if (currentTimerValue >= mPeriod / 2 - 1)
            {
              // The timer is a little behind, so kick it forward
              //TIM2->CNT = mPeriod - 1;
              bsp_set_sotdma_timer_value(mPeriod - 1);
            }
          else
            {
              // The timer is a little ahead, so pull it back
              //TIM2->CNT = 0;
              bsp_set_sotdma_timer_value(0);
            }
        }
    }

  Event *e = EventPool::instance().newEvent(CLOCK_EVENT);
  if ( e )
    {
      e->clock.utc = mUTC;
      EventQueue::instance ().push(e);
    }
}


void GPS::processEvent(const Event &event)
{
  processLine(event.nmeaBuffer.sentence);
  ASSERT(event.rxPacket == nullptr);
}

void GPS::processLine(const char* buff)
{
  if ( buff[0] == '$' && buff[1] != '$' )
    {
      unsigned reportedHash;
      char *starPos = strstr(buff, "*");
      if ( starPos && sscanf(starPos + 1, "%x", &reportedHash) == 1 )
        {
          unsigned actualHash = 0;
          for ( const char* c = buff + 1; c < starPos; ++c )
            actualHash ^= *c;

          if ( reportedHash == actualHash )
            {
              parseSentence(buff);
            }

        }
    }
}

void GPS::parseSentence(const char *buff)
{
  NMEASentence sentence (buff);
#ifdef MULTIPLEXED_OUTPUT
  DataTerminal::instance ().write ("NMEA", buff);
#else
  DataTerminal::instance().write(buff);
#endif

  if (sentence.code ().find ("RMC") == 2)
    {
      const vector<string> &fields = sentence.fields ();

      /*
       * Sometimes the GPS indicates errors with sentences like
       * $GPRMC,1420$0*74\r\n
       * Although the sentence structure is valid, its content is not what we expect.
       *
       * TODO: Should we consider the GPS non-functioning at this point and thus prevent transmission until it recovers?
       */
      if (fields.size () < 10)
        return;

      // GPS updates arrive even with no time or fix information, so ignore them if that's the case
      if (fields[1].length () < 6 || fields[9].length () < 6)
        {
          // TODO: A loss of fix while the SOTDMA timer is active, MUST stop the timer
          bsp_gps_led_off();
          return;
        }


      // This is the time that corresponds to the previous PPS
      const string &timestr = fields[1].substr (0, 6);
      const string &datestr = fields[9].substr (0, 6);
      mTime.tm_hour = Utils::toInt (timestr.substr (0, 2));
      mTime.tm_min = Utils::toInt (timestr.substr (2, 2));
      mTime.tm_sec = Utils::toInt (timestr.substr (4, 2));
      mTime.tm_mday = Utils::toInt (datestr.substr (0, 2));
      mTime.tm_mon = Utils::toInt (datestr.substr (2, 2)) - 1; // Month is 0-based
      mTime.tm_year = 100 + Utils::toInt (datestr.substr (4, 2)); // Year is 1900-based
      mUTC = mktime (&mTime);

      // Do we have a fix?
      if (mUTC && sentence.fields()[3].length() > 0 && sentence.fields()[5].length() > 0)
        {
          bsp_gps_led_on();
          mLat = Utils::latitudeFromNMEA (sentence.fields()[3], sentence.fields()[4]);
          mLng = Utils::longitudeFromNMEA (sentence.fields()[5], sentence.fields()[6]);
          mSpeed = atof(sentence.fields()[7].c_str());
          mCOG = atof(sentence.fields()[8].c_str());
          Event *e = EventPool::instance().newEvent(GPS_FIX_EVENT);
          if ( e )
            {
              Event &event = *e;
              event.gpsFix.utc = mUTC;
              event.gpsFix.lat = mLat;
              event.gpsFix.lng = mLng;
              event.gpsFix.speed = mSpeed;
              event.gpsFix.cog = mCOG;
              EventQueue::instance().push (e);
            }
        }
      else
        {
          bsp_gps_led_off();
        }
    }
}


void GPS::startTimer()
{
  bsp_start_sotdma_timer();
  mStarted = true;
  DBG("Started SOTDMA timer\r\n");
}

void GPS::stopTimer()
{
  mStarted = false;
  bsp_stop_sotdma_timer();
  DBG("Stopped SOTDMA timer\r\n");
}


void GPS::onTimerIRQ()
{
  if ( mStarted )
    {
      ++mSlotNumber;
      if ( mSlotNumber == 2250 )
        mSlotNumber = 0;

      // Delegates need real-time information
      if ( mDelegate )
        mDelegate->timeSlotStarted(mSlotNumber);
    }
}

void gnssInputCB(char c)
{
  GPS::instance().onRX(c);
}

void gnss1PPSCB()
{
  GPS::instance().onPPS();
}

void gnssSOTDMACB()
{
  GPS::instance().onTimerIRQ();
}



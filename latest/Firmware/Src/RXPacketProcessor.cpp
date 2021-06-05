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


#include <stdio.h>
#include "_assert.h"
#include "config.h"
#include "RadioManager.hpp"
#include "RXPacketProcessor.hpp"
#include "AISMessages.hpp"
#include "DataTerminal.hpp"
#include "EventQueue.hpp"
#include "Utils.hpp"
#include "AISChannels.h"
#include "printf_serial.h"
#include "bsp.hpp"


#if MULTIPLEXED_OUTPUT
char __buff[120];
#endif

RXPacketProcessor::RXPacketProcessor ()
{
  mSentences.reserve(4); // We're not going to need more than 2 sentences for the longest AIS message we report ...
  Configuration::instance().readStationData(mStationData);
  EventQueue::instance().addObserver(this, AIS_PACKET_EVENT);
}

RXPacketProcessor::~RXPacketProcessor ()
{
  // Should never be called
  EventQueue::instance().removeObserver(this);
}

void RXPacketProcessor::processEvent(const Event &e)
{
  switch(e.type)
  {
  case AIS_PACKET_EVENT:
    {
      ASSERT(e.rxPacket);
      if (e.rxPacket->isBad() || !e.rxPacket->checkCRC ())
        return;

      bsp_rx_led_on();

      if ( e.rxPacket->messageType() == 15 )
        {
          AISMessage15 msg;
          if ( msg.decode(*e.rxPacket) )
            {
              // Make sure we actually can transmit something
              if ( mStationData.magic != STATION_DATA_MAGIC )
                break;

              // This is an interrogation. If we are a target, push an appropriate event into the queue

              // It is possible that we are the target for more than one type of message (18 + 24)
              for ( uint8_t i = 0; i < 3; ++i )
                {
                  AISMessage15::InterrogationTarget &target = msg.targets[i];
                  if ( target.mmsi == mStationData.mmsi )
                    {
                      switch(target.messageType)
                      {
                      case 18:
                      case 24:
                        {
#if 0
                          Event ie(INTERROGATION_EVENT);
                          ie.interrogation.channel = e.rxPacket.channel();
                          ie.interrogation.messageType = target.messageType;

                          //printf2("Scheduling message %d in response to interrogation\r\n", ie->interrogation.messageType);
                          EventQueue::instance().push(ie);
#endif
                          break;
                        }
                      default:
                        // Why do base stations sometimes request message 5 from class B transponders?
                        //DBG("Ignoring malformed message 15 from MMSI %d\r\n", e.rxPacket.mmsi());
                        break;
                      }
                    }
                }

            }
        } // If message 15


      mSentences.clear();

      ASSERT_VALID_PTR(e.rxPacket);
      mEncoder.encode(*(e.rxPacket), mSentences);
      for (vector<string>::iterator i = mSentences.begin(); i != mSentences.end(); ++i)
        {
#ifdef MULTIPLEXED_OUTPUT
          sprintf(__buff, "%s\r\n", i->c_str());
          DataTerminal::instance().write("NMEA", __buff);
#else
          DataTerminal::instance().write(i->c_str());
          DataTerminal::instance().write("\r\n");
#endif
        }


      // Special handling for specific messages that we care about
      switch (e.rxPacket->messageType())
      {
      case 20:
        // TODO: This is a time slot reservation from a base station. Use this information to block those time slots.
        break;
      case 22:
        /*
           TODO:

           This is the frequency management message. Since we support all of the upper VHF band (161.500 MHz - 162.025 MHz),
           we use this to switch our primary 2 channels where instructed. That said, I'm not completely clear on how channels
           are described in this message. The spec references ITU-R M.1084 and allocates 12 bits for the channel number,
           but whoever wrote this document did not care about comprehension :(

           https://www.itu.int/dms_pubrec/itu-r/rec/m/R-REC-M.1084-5-201203-I!!PDF-E.pdf
         */

        break;
      case 23:
        /*
           TODO: This is the group assignment message. Base stations can use this to configure our transmission interval
                 or enforce silent periods as well. We should comply.
         */
        break;
      }



      break;
    }
  default:
    break;
  }

}

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


#include "NMEAEncoder.hpp"
#include "Utils.hpp"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include "AISChannels.h"

using namespace std;


NMEAEncoder::NMEAEncoder()
: mSequence(0)
{
}

NMEAEncoder::~NMEAEncoder()
{
}

void NMEAEncoder::encode(RXPacket &packet, vector<string> &sentences)
{
  static uint16_t MAX_SENTENCE_BYTES = 56;
  static uint16_t MAX_SENTENCE_BITS = MAX_SENTENCE_BYTES * 6;

  packet.discardCRC();

  uint16_t numBits = packet.size();
  uint16_t fillBits = 0;

  if ( numBits % 6 )
    {
      fillBits = 6 - (numBits%6);
      packet.addFillBits(fillBits);
      numBits = packet.size();
    }

  uint16_t numSentences = 1;
  while ( numBits > MAX_SENTENCE_BITS )
    {
      ++numSentences;
      numBits -= MAX_SENTENCE_BITS;
    }


  numBits = packet.size();
  if ( numSentences > 1 )
    {
      ++mSequence;

      if ( mSequence > 9 )
        mSequence = 0;
    }


  // Now we know how many sentences we need
  char sentence[85];
  uint16_t pos = 0;

  for ( uint16_t i = 1; i <= numSentences; ++i )
    {
      uint8_t k = 0;
      if ( numSentences > 1 )
        sprintf(sentence, "!AIVDM,%d,%d,%d,%c,", numSentences, i, mSequence, AIS_CHANNELS[packet.channel()].designation);
      else
        sprintf(sentence, "!AIVDM,%d,%d,,%c,", numSentences, i, AIS_CHANNELS[packet.channel()].designation);

      k = strlen(sentence);
      uint16_t sentenceBits = 0;

      for ( ; pos < numBits && sentenceBits < MAX_SENTENCE_BITS; pos += 6, sentenceBits += 6 )
        {
          uint8_t nmeaByte = (uint8_t)packet.bits(pos, 6);
          nmeaByte += (nmeaByte < 40) ? 48 : 56;
          sentence[k++] = nmeaByte;
        }

      sentence[k++] = ',';
      if ( numSentences > 1 )
        {
          if ( i == numSentences )
            sentence[k++] = '0' + fillBits;
          else
            sentence[k++] = '0';
        }
      else
        {
          sentence[k++] = '0' + fillBits;
        }

      sentence[k++] = '*';
      sprintf(sentence+k, "%.2X", nmeaCRC(sentence));
      sentences.push_back(string(sentence));
    }

}

uint8_t NMEAEncoder::nmeaCRC(const char* buff)
{
  uint8_t p = 1;
  uint8_t crc = buff[p++];
  while ( buff[p] != '*' )
    crc ^= buff[p++];
  return crc;
}


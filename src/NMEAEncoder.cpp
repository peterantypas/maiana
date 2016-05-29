/*
 * NMEAEncoder.cpp
 *
 *  Created on: Jan 18, 2016
 *      Author: peter
 */

#include "NMEAEncoder.hpp"
#include "Utils.hpp"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <diag/Trace.h>
#include "AISChannels.h"

using namespace std;


NMEAEncoder::NMEAEncoder()
    : mSequence(0)
{
}

NMEAEncoder::~NMEAEncoder()
{
}

void NMEAEncoder::encode(RXPacket &packet, list<string> &sentences)
{
    static uint16_t MAX_SENTENCE_BYTES = 56;
    static uint16_t MAX_SENTENCE_BITS = MAX_SENTENCE_BYTES * 6;

    packet.discardCRC();

    uint16_t numBits = packet.size();
    uint16_t fillBits = 0;

    if ( numBits % 6 ) {
        fillBits = 6 - (numBits%6);
        packet.addFillBits(fillBits);
        numBits = packet.size();
    }

    uint16_t numSentences = 1;
    while ( numBits > MAX_SENTENCE_BITS ) {
        ++numSentences;
        numBits -= MAX_SENTENCE_BITS;
    }


    numBits = packet.size();
    if ( numSentences > 1 ) {
        ++mSequence;

        if ( mSequence > 9 )
            mSequence = 0;
    }


    // Now we know how many sentences we need
    char sentence[85];
    uint16_t pos = 0;

    for ( uint16_t i = 1; i <= numSentences; ++i ) {
        uint8_t k = 0;
        if ( numSentences > 1 )
            sprintf(sentence, "!AIVDM,%d,%d,%d,%c,", numSentences, i, mSequence, AIS_CHANNELS[packet.channel()].designation);
        else
            sprintf(sentence, "!AIVDM,%d,%d,,%c,", numSentences, i, AIS_CHANNELS[packet.channel()].designation);

        k = strlen(sentence);
        uint16_t sentenceBits = 0;

        for ( ; pos < numBits && sentenceBits < MAX_SENTENCE_BITS; pos += 6, sentenceBits += 6 ) {
            uint8_t nmeaByte = (uint8_t)packet.bits(pos, 6);
            nmeaByte += (nmeaByte < 40) ? 48 : 56;
            sentence[k++] = nmeaByte;
        }

        //trace_printf("Sentence bits: %d\n", sentenceBits);


        sentence[k++] = ',';
        if ( numSentences > 1 ) {
            if ( i == numSentences )
                sentence[k++] = '0' + fillBits;
            else
                sentence[k++] = '0';
        }
        else
            sentence[k++] = '0';

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


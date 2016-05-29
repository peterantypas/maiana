/*
 * NMEAEncoder.hpp
 *
 *  Created on: Jan 18, 2016
 *      Author: peter
 */

#ifndef NMEAENCODER_HPP_
#define NMEAENCODER_HPP_

#include <list>
#include <string>
#include "RXPacket.hpp"
using namespace std;


class NMEAEncoder
{
public:
    NMEAEncoder();
    virtual ~NMEAEncoder();

    void encode(RXPacket &packet, list<string> &sentences);
private:
    uint8_t nmeaCRC(const char* buff);
private:
    uint8_t mSequence;
};

#endif /* NMEAENCODER_HPP_ */

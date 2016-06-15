/*
 * AISMessages.hpp
 *
 *  Created on: Dec 12, 2015
 *      Author: peter
 */

#ifndef AISMESSAGES_HPP_
#define AISMESSAGES_HPP_

#include "TXPacket.hpp"
#include "RXPacket.hpp"
#include <time.h>
#include <string>
#include "StationData.h"


using namespace std;

// These are the AIS messages that this unit will actually work with


// For message 18 -- all class B "CS" stations send this
#define DEFAULT_COMM_STATE 0b1100000000000000110

class AISMessage
{
public:
    AISMessage ();
    virtual ~AISMessage ();

    virtual bool decode(const RXPacket &packet);
    virtual void encode(const StationData &station, TXPacket &packet);

    uint8_t type() const;
    uint8_t repeatIndicator() const;
    uint32_t mmsi() const;
protected:
    // Every AIS message has these attributes at a minimum
    uint8_t mType;
    uint8_t mRI;
    uint32_t mMMSI;
protected:

    void appendCRC(uint8_t *buff, uint16_t &size);
    void addBits(uint8_t *buff, uint16_t &size, uint32_t value, uint8_t numBits);
    void putBits(uint8_t *buff, uint32_t value, uint8_t numBits);
    void addString(uint8_t *buff, uint16_t &size, const string &name, uint8_t maxChars);
    void finalize(uint8_t *buff, uint16_t &size, TXPacket &packet);
private:
    void bitStuff(uint8_t *buff, uint16_t &size);
    void constructHDLCFrame(uint8_t *buff, uint16_t &size);
    void nrziEncode(uint8_t *buff, uint16_t &size, TXPacket &packet);
    void payloadToBytes(uint8_t *bitVector, uint16_t numBits, uint8_t *bytes);
    void reverseEachByte(uint8_t *bitVector, uint16_t numBits);
};

class AISMessage123 : public AISMessage
{
public:
    double latitude;
    double longitude;
    double sog;
    double cog;

    AISMessage123();

    bool decode(const RXPacket &packet);
};

class AISMessage18 : public AISMessage
{
public:
    double latitude;
    double longitude;
    double sog;
    double cog;
    time_t utc;

    AISMessage18();

    bool decode(const RXPacket &packet);
    void encode(const StationData &data, TXPacket &packet);
};

class AISMessage24A : public AISMessage
{
public:
    //string name;
    AISMessage24A();

    void encode(const StationData &data, TXPacket &packet);
};

class AISMessage24B : public AISMessage
{
public:
    AISMessage24B();

    //string vendorId;
    //string callSign;

    void encode(const StationData &data, TXPacket &packet);

};


#endif /* AISMESSAGES_HPP_ */

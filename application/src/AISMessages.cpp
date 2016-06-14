/*
 * AISMessages.cpp
 *
 *  Created on: Dec 12, 2015
 *      Author: peter
 */

#include <cmath>
#include "AISMessages.hpp"
#include "Utils.hpp"
#include <cassert>
#include <cstring>
#include <sstream>
#include <diag/Trace.h>

#define USE_MSB

// A good quick reference for AIS message format: http://www.navcen.uscg.gov/?pageName=AISMessages

AISMessage::AISMessage ()
{
    mMMSI = 0;
    mRI = 0;
    mType = 0;
}

AISMessage::~AISMessage ()
{
}

uint8_t AISMessage::type() const
{
    return mType;
}

uint8_t AISMessage::repeatIndicator() const
{
    return mRI;
}

uint32_t AISMessage::mmsi() const
{
    return mMMSI;
}

bool AISMessage::decode(RXPacket &)
{
    // The base class method should never be called
    ASSERT(false);

    // This will never execute but the compiler is not smart enough to know it
    return false;
}

void AISMessage::encode(const StationData &station, TXPacket &)
{
    mMMSI = station.mmsi;
}

void AISMessage::addBits(uint8_t *bitVector, uint16_t &size, uint32_t value, uint8_t numBits)
{
    ASSERT(numBits > 0  && numBits <= 32);
    uint16_t pos = size;
    for ( uint8_t bit = 0; bit < numBits; ++bit, value >>= 1 )  {
        bitVector[pos + numBits-bit-1] = value & 1;
    }

    size += numBits;
}

void AISMessage::putBits(uint8_t *bitVector, uint32_t value, uint8_t numBits)
{
    // This is used for HDLC framing
    uint16_t pos = 0;
    for ( uint8_t bit = 0; bit < numBits; ++bit, value >>= 1 )  {
        bitVector[pos++] = value & 0x01;
    }
}

void AISMessage::reverseEachByte(uint8_t *bitVector, uint16_t size)
{
    ASSERT(size % 8 == 0);
    for ( uint16_t i = 0; i < size; i += 8 ) {
        for ( uint8_t j = 0; j < 4; ++j ) {
            swap(bitVector[i+j], bitVector[i+7-j]);
        }
    }
}

void AISMessage::addString(uint8_t *bitVector, uint16_t &size, const string &value, uint8_t maxChars)
{
    ASSERT(value.length() <= maxChars);
    ASSERT(maxChars < 30); // There should be no application for such long strings here
    char s[30];
    memset(s, 0, sizeof s);
    strncpy(s, value.c_str(), value.length());

    uint8_t buffer[32];
    for ( uint8_t c = 0; c < maxChars; ++c ) {
        uint8_t byte = s[c] >= 64 ? s[c]-64 : s[c];
        buffer[c] = byte;
    }

    for ( uint8_t c = 0; c < maxChars; ++c )
        addBits(bitVector, size, buffer[c], 6);
}

void AISMessage::payloadToBytes(uint8_t *bitVector, uint16_t numBits, uint8_t *byteArray)
{
    for ( uint16_t i = 0; i < numBits; i += 8 ) {
        uint8_t byte = 0;
        for ( uint8_t b = 0; b < 8; ++b ) {
            byte |= (bitVector[i+b] << b);
        }
        byteArray[i/8] = byte;
    }
}

void AISMessage::finalize(uint8_t *payload, uint16_t &size, TXPacket &packet)
{
    // Nothing we send exceeds 256 bits, including preambles and such. 40 bytes is more than enough.
    uint8_t bytes[40];

    // CRC-CCITT calculation
    payloadToBytes(payload, size, bytes);
    uint16_t crc = Utils::crc16(bytes, size/8);
    uint8_t crcL = crc & 0x00ff;
    uint8_t crcH = (crc & 0xff00) >> 8;
    addBits(payload, size, crcL, 8);
    addBits(payload, size, crcH, 8);
    payloadToBytes(payload, size, bytes);

    // Encoding for transmission
    reverseEachByte(payload, size);
    bitStuff(payload, size);
    constructHDLCFrame(payload, size);
    nrziEncode(payload, size, packet);
    packet.pad();
}

void AISMessage::bitStuff(uint8_t *buff, uint16_t &size)
{
    uint16_t numOnes = 0;
    for ( uint16_t i = 0; i < size; ++i ) {
        switch(buff[i]) {
            case 0:
                numOnes = 0;
                break;
            case 1:
                ++numOnes;
                if ( numOnes == 5 ) {
                    // Insert a 0 right after this one
                    memmove(buff + i + 2, buff + i + 1, size-i-1);
                    buff[i+1] = 0;
                    ++size;
                }
                break;
            default:
                ASSERT(false);
        }
    }
}

void AISMessage::constructHDLCFrame(uint8_t *buff, uint16_t &size)
{
    /*
     * As a class B "CS" transponder, we don't transmit a full ramp byte because
     * we have to listen for a few bits into each slot for Clear Channel Assessment.
     * Also, this implementation only adds 3 bits for ramp down, just in case
     * our TX bit clock is a little lazy. Not what the ITU standard says, but no
     * reasonable receiver should care about ramp-down bits. It's only what goes
     * between the 0x7E markers that counts.
     */

    // Make room for 35 bits at the front
    memmove(buff+35, buff, size);
    size += 35;
    putBits(buff, 0xFF, 3);                             // 3 ramp bits. That's all we can afford.
    putBits(buff+3, 0b010101010101010101010101, 24);    // 24 training bits
    putBits(buff+27, 0x7e, 8);                          // HDLC start flag

    // Now append the end marker and ramp-down bits
    addBits(buff, size, 0x7e, 8);                       // HDLC stop flag
    addBits(buff, size, 0x00, 3);                       // Ramp down
}

void AISMessage::nrziEncode(uint8_t *buff, uint16_t &size, TXPacket &packet)
{
    uint8_t prevBit = 1;        // Arbitrarily starting with 1
    packet.addBit(prevBit);

    for ( uint16_t i = 0; i < size; ++i ) {
        if ( buff[i] == 0 ) {
            packet.addBit(!prevBit);
            prevBit = !prevBit;
        }
        else {
            packet.addBit(prevBit);
        }
    }

    // The TXPacket is now populated with the sequence of bits that need to be sent
}

///////////////////////////////////////////////////////////////////////////////
//
// AISMessage123
//
///////////////////////////////////////////////////////////////////////////////
AISMessage123::AISMessage123()
{
}

bool AISMessage123::decode(RXPacket &packet)
{
    mType = packet.messageType();
    mRI = packet.repeatIndicator();
    mMMSI = packet.mmsi();
    sog = packet.bits(50, 10) / 10.0f;
    longitude = Utils::coordinateFromUINT32(packet.bits(61, 28), 28);
    latitude = Utils::coordinateFromUINT32(packet.bits(89, 27), 27);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// AISMessage18
//
///////////////////////////////////////////////////////////////////////////////

AISMessage18::AISMessage18()
{
    mType = 18;
}

void AISMessage18::encode(const StationData &station, TXPacket &packet)
{
    AISMessage::encode(station, packet);

    // TODO: Perhaps this shouldn't live on the stack?
    uint8_t payload[MAX_AIS_TX_PACKET_SIZE];
    uint16_t size = 0;
    uint32_t value;

    value = mType;
    addBits(payload, size, value, 6);   // Message type

    value = mRI;
    addBits(payload, size, value, 2);   // Repeat Indicator

    value = mMMSI;
    addBits(payload, size, value, 30);  // MMSI

    value = 0;
    addBits(payload, size, value, 8);   // Spare bits

    value = (uint32_t)(sog * 10);
    addBits(payload, size, value, 10);  // Speed (knots x 10)

    value = 1;
    addBits(payload, size, value, 1);   // Position accuracy is high

    value = Utils::coordinateToUINT32(longitude);
    addBits(payload, size, value, 28);  // Longitude

    value = Utils::coordinateToUINT32(latitude);
    addBits(payload, size, value, 27);  // Latitude

    value = (uint32_t)(cog * 10);
    addBits(payload, size, value, 12);  // COG

    value = 511;
    addBits(payload, size, value, 9);   // We don't know our heading

    value = utc % 60;
    addBits(payload, size, value, 6);   // UTC second. Possibly incorrect.

    value = 0;
    addBits(payload, size, value, 2);   // Spare

    value = 1;
    addBits(payload, size, value, 1);   // We are a "CS" unit

    value = 0;
    addBits(payload, size, value, 1);   // We have no display

    value = 0;
    addBits(payload, size, value, 1);   // We have no DSC

    value = 0;
    addBits(payload, size, value, 1);   // We don't switch frequencies so this doesn't matter

    value = 0;
    addBits(payload, size, value, 1);   // We do not respond to message 22 to switch frequency

    value = 0;
    addBits(payload, size, value, 1);   // We operate in autonomous and continuous mode

    value = 0;
    addBits(payload, size, value, 1);   // No RAIM

    value = 1;
    addBits(payload, size, value, 1);   // We use ITDMA (as a CS unit)

    value = DEFAULT_COMM_STATE;
    addBits(payload, size, value, 19);  // Standard communication state flag for CS units

#if DEV_MODE
    ASSERT(size == 168);
#endif

    finalize(payload, size, packet);
}

bool AISMessage18::decode(RXPacket &packet)
{
    mType = packet.messageType();
    mRI = packet.repeatIndicator();
    mMMSI = packet.mmsi();

    sog = packet.bits(46, 10) / 10.0f;
    longitude = Utils::coordinateFromUINT32(packet.bits(57, 28), 28);
    latitude = Utils::coordinateFromUINT32(packet.bits(85, 27), 27);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// AISMessage24A
//
///////////////////////////////////////////////////////////////////////////////
AISMessage24A::AISMessage24A()
{
    mType = 24;
}

void AISMessage24A::encode(const StationData &station, TXPacket &packet)
{
    AISMessage::encode(station, packet);

    uint8_t payload[MAX_AIS_TX_PACKET_SIZE];
    uint16_t size = 0;
    uint32_t value;

    value = mType;
    addBits(payload, size, value, 6);   // Message type

    value = mRI;
    addBits(payload, size, value, 2);   // Repeat Indicator

    value = mMMSI;
    addBits(payload, size, value, 30);  // MMSI

    value = 0;
    addBits(payload, size, value, 2);   // Part number (0 for 24A)
    addString(payload, size, station.name, 20); // Station name

    finalize(payload, size, packet);
}

///////////////////////////////////////////////////////////////////////////////
//
// AISMessage24B
//
///////////////////////////////////////////////////////////////////////////////
AISMessage24B::AISMessage24B()
{
    mType = 24;
}

void AISMessage24B::encode(const StationData &station, TXPacket &packet)
{
    AISMessage::encode(station, packet);

    uint8_t payload[MAX_AIS_TX_PACKET_SIZE];
    uint16_t size = 0;
    uint32_t value;

    value = mType;
    addBits(payload, size, value, 6);   // Message type

    value = mRI;
    addBits(payload, size, value, 2);   // Repeat Indicator

    value = mMMSI;
    addBits(payload, size, value, 30);  // MMSI

    value = 1;
    addBits(payload, size, value, 2);   // Part number (1 for 24B)

    value = 0;
    addBits(payload, size, value, 8);   // Type of ship unknown

    addString(payload, size, "", 7);
    addString(payload, size, station.callsign, 7);

    value = 0;
    addBits(payload, size, value, 30);  // No dimension information

    value = 1;
    addBits(payload, size, value, 4);   // Using GPS

    value = 0;
    addBits(payload, size, value, 2);   // Spare bits

    finalize(payload, size, packet);
}




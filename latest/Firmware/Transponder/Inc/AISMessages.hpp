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

#ifndef AISMESSAGES_HPP_
#define AISMESSAGES_HPP_

#include "TXPacket.hpp"
#include "RXPacket.hpp"
#include <time.h>
#include <string>
#include "StationData.h"


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

#if 0
class AISMessage123 : public AISMessage
{
public:
  float latitude;
  float longitude;
  float sog;
  float cog;

  AISMessage123();

  bool decode(const RXPacket &packet);
};
#endif

/**
 * We may receive message 15 (interrogation) from a shore station, so we need to decode these
 */
class AISMessage15: public AISMessage
{
public:
  typedef struct {
    uint32_t mmsi;
    uint8_t  messageType;
  } InterrogationTarget;

  InterrogationTarget targets[3];

  AISMessage15();

  bool decode(const RXPacket &packet);
};

/**
 * Message 18 is our position report (as a class B). We transmit this.
 */
class AISMessage18 : public AISMessage
{
public:
  float latitude;
  float longitude;
  float sog;
  float cog;
  time_t utc;

  AISMessage18();

  //bool decode(const RXPacket &packet);
  void encode(const StationData &data, TXPacket &packet);
};

/**
 * Message 24 part A is part of our station metadata. We transmit this.
 */
class AISMessage24A : public AISMessage
{
public:
  AISMessage24A();

  void encode(const StationData &data, TXPacket &packet);
};

/**
 * Message 24 part B is additional station metadata. We transmit this.
 */
class AISMessage24B : public AISMessage
{
public:
  AISMessage24B();

  void encode(const StationData &data, TXPacket &packet);
};


#endif /* AISMESSAGES_HPP_ */

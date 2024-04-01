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

#include <stm32l4xx_hal.h>
#include "Configuration.hpp"
#include "Utils.hpp"
#include "config.h"
#include "EventQueue.hpp"
#include <stdio.h>
#include <bsp/bsp.hpp>


// These are not defined in ANY CMSIS or HAL header, WTF???
#define OTP_ADDRESS           0x1FFF7000
#define OTP_SIZE              0x00000400


#define CONFIG_FLAGS_MAGIC    0x2092ED2C
#define XO_TRIM_MAGIC         0x8112AAAA


//static StationDataPage __page;

Configuration &Configuration::instance()
{
  static Configuration __instance;
  return __instance;
}


Configuration::Configuration()
{
}

void Configuration::init()
{
  bool cliBootMode = *(uint32_t*)BOOTMODE_ADDRESS == CLI_FLAG_MAGIC;
  if ( !cliBootMode )
    {
      reportSystemData();
      reportStationData();
    }

  bsp_read_config_flags(&mFlags);
}

void Configuration::enableTX()
{
  // For now, the only flag in use is a TX switch bit which is set to 0
  mFlags = {CONFIG_FLAGS_MAGIC, 0, {0}};
  bsp_write_config_flags(mFlags);
}

void Configuration::disableTX()
{
  // For now, the only flag in use is a TX switch bit which is set to 1
  mFlags = {CONFIG_FLAGS_MAGIC, 0, {0}};
  mFlags.flags[0] = 0x01;
  bsp_write_config_flags(mFlags);
}

bool Configuration::isTXEnabled()
{
  if ( mFlags.magic != CONFIG_FLAGS_MAGIC )
    return true;

  // Bit 0 in word 0 inhibits transmission
  return (mFlags.flags[0] & 0x01) == 0;
}

const char *Configuration::hwRev()
{
  const OTPData *otp = readOTP();
  if ( otp )
    return otp->hwrev;
  else
    return BSP_HW_REV;
}

const char *Configuration::serNum()
{
  const OTPData *otp = readOTP();
  if ( otp )
    return otp->serialnum;
  else
    return "";
}

const char *Configuration::mcuType()
{
#ifdef STM32L422xx
  return "STM32L422";
#elif defined STM32L432xx
  return "STM32L432";
#elif defined STM32L412xx
  return "STM32L412";
#else
    return "";
#endif
}

const char *Configuration::breakoutType()
{
#if LEGACY_BREAKOUTS == 0
  return "1";
#else
  return "0";
#endif
}

bool Configuration::isBootloaderPresent()
{
#ifndef VECT_TAB_OFFSET
  return false;
#else
  return true;
#endif
}

void Configuration::reportSystemData()
{
  Event *e = EventPool::instance().newEvent(PROPR_NMEA_SENTENCE);
  if ( !e )
    return;

  sprintf(e->nmeaBuffer.sentence, "$PAISYS,%s,%s,%s,%s,%s,%d*", hwRev(), FW_REV, serNum(), mcuType(),
      breakoutType(), isBootloaderPresent());

  Utils::completeNMEA(e->nmeaBuffer.sentence);
  EventQueue::instance().push(e);
}

void Configuration::reportStationData()
{
  StationData d;
  if ( !readStationData(d) )
    memset(&d, 0, sizeof d);

  Event *e = EventPool::instance().newEvent(PROPR_NMEA_SENTENCE);
  if ( !e )
    return;

  sprintf(e->nmeaBuffer.sentence,
      "$PAISTN,%lu,%s,%s,%d,%d,%d,%d,%d*",
      d.mmsi,
      d.name,
      d.callsign,
      d.type,
      d.len,
      d.beam,
      d.portOffset,
      d.bowOffset);

  Utils::completeNMEA(e->nmeaBuffer.sentence);
  EventQueue::instance().push(e);
}

bool Configuration::isStationDataProvisioned()
{
  return bsp_is_station_data_provisioned();
}

void Configuration::eraseStationData()
{
  bsp_erase_station_data();
}

#if OTP_DATA
void Configuration::reportOTPData()
{
  const OTPData *data = readOTP();
  Event *e = EventPool::instance().newEvent(PROPR_NMEA_SENTENCE);
  if ( !e )
    return;

  if ( data )
    {
      sprintf(e->nmeaBuffer.sentence, "$PAIOTP,%s,%s*", data->serialnum, data->hwrev);
    }
  else
    {
      strcpy(e->nmeaBuffer.sentence, "$PAIOTP,,*");
    }
  Utils::completeNMEA(e->nmeaBuffer.sentence);
  EventQueue::instance().push(e);

}
#endif

void Configuration::reportXOTrimValue()
{
  uint8_t value = 0;

  if ( this->isXOTrimmed() )
    {
      value = this->getXOTrimValue();
    }

  Event *e = EventPool::instance().newEvent(PROPR_NMEA_SENTENCE);
  if ( !e )
    return;

  sprintf(e->nmeaBuffer.sentence, "$PAIXOTR,%d*", value);
  Utils::completeNMEA(e->nmeaBuffer.sentence);
  EventQueue::instance().push(e);
}

void Configuration::factoryReset()
{
  bsp_erase_station_data();
  mFlags = {0};
  bsp_erase_config_flags();
  bsp_erase_xo_trim();
}

bool Configuration::writeStationData(const StationData &data)
{
  bsp_write_station_data(data);
  reportStationData();
  return true;
}

bool Configuration::readStationData(StationData &data)
{
  bsp_read_station_data(&data);
  return data.magic == STATION_DATA_MAGIC;
}


bool Configuration::isXOTrimmed()
{
  XOTrim x;
  bsp_read_xo_trim(&x);
  return x.magic == XO_TRIM_MAGIC;
}

void Configuration::setXOTrimValue(uint8_t value)
{
  XOTrim x = {XO_TRIM_MAGIC, value, {0}};
  bsp_write_xo_trim(x);
}

uint8_t Configuration::getXOTrimValue()
{
  XOTrim x;
  bsp_read_xo_trim(&x);
  return x.value;
}


#if OTP_DATA
const OTPData *Configuration::readOTP()
{
  uint32_t address = nextAvailableOTPSlot();
  if ( address == OTP_ADDRESS )
    // There's nothing written!
    return nullptr;

  address -= sizeof(OTPData);
  if ( IS_FLASH_OTP_ADDRESS(address) )
    return (const OTPData*)address;

  return nullptr;
}

bool Configuration::writeOTP(const OTPData &data)
{
  uint32_t address = nextAvailableOTPSlot();
  if ( !IS_FLASH_OTP_ADDRESS(address) )
    return false;

  uint64_t *d = (uint64_t*)&data;

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
  HAL_FLASH_Unlock();
  HAL_StatusTypeDef status = HAL_OK;
  for ( uint32_t dw = 0; dw < sizeof data/8; ++dw, ++d )
    {
      status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + dw*8, *d);
      if ( status != HAL_OK )
        break;
    }
  HAL_FLASH_Lock();

  return true;
}

uint32_t Configuration::nextAvailableOTPSlot()
{
  for ( uint32_t p = OTP_ADDRESS; p < OTP_ADDRESS+OTP_SIZE; p += sizeof (OTPData) )
    {
      OTPData *d = (OTPData*)p;
      if ( d->magic == 0xFFFFFFFF )
        return p;
    }

  return OTP_ADDRESS+OTP_SIZE;
}
#endif




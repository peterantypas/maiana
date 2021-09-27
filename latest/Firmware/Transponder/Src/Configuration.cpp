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


// These are not defined in ANY CMSIS or HAL header, WTF ST???
#define OTP_ADDRESS   0x1FFF7000
#define OTP_SIZE      0x00000400

#define CONFIG_FLAGS_MAGIC    0x2092ED2C

#if 0
static StationData __THIS_STATION__ = {
    STATION_DATA_MAGIC,
    987654321,                // MMSI
    "NAUT",                   // Name
    "",                       // Call sign
    0,                        // Length overall
    0,                        // Beam
    0,                        // Bow offset
    0,                        // Port offset
    VESSEL_TYPE_UNSPECIFIED
};
#endif

static StationDataPage __page;

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
}

void Configuration::enableTX()
{
  // For now, the only flag in use is a TX switch bit which is set to 0
  ConfigFlags flags = {CONFIG_FLAGS_MAGIC, 0, {0}};
  writeConfigFlags(&flags);
}

void Configuration::disableTX()
{
  // For now, the only flag in use is a TX switch bit which is set to 0
  ConfigFlags flags = {CONFIG_FLAGS_MAGIC, 0, {0}};
  flags.flags[0] = 0x01;
  writeConfigFlags(&flags);
}

bool Configuration::isTXEnabled()
{
  const ConfigFlags *cfg = readConfigFlags();
  if ( cfg == nullptr )
    return true;

  // Bit 0 in word 0 inhibits transmission
  return (cfg->flags[0] & 0x01) == 0;
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

void Configuration::reportSystemData()
{
  Event *e = EventPool::instance().newEvent(PROPR_NMEA_SENTENCE);
  if ( !e )
    return;

  sprintf(e->nmeaBuffer.sentence, "$PAISYS,%s,%s,%s*", hwRev(), FW_REV, serNum());

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
  StationData d;
  return readStationData(d);
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

bool Configuration::eraseStationDataPage()
{
  return erasePage(STATION_DATA_ADDRESS);
}

//__attribute__ ((long_call, section (".code_in_ram")))
bool Configuration::erasePage(uint32_t address)
{
  uint32_t page = (address - FLASH_BASE) / FLASH_PAGE_SIZE;

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
  if ( HAL_FLASH_Unlock() != HAL_OK )
    return false;

  FLASH_EraseInitTypeDef erase;
  erase.TypeErase = FLASH_TYPEERASE_PAGES;
  erase.Banks     = FLASH_BANK_1;
  erase.Page      = page;
  erase.NbPages   = 1;

  uint32_t errPage;
  HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &errPage);
  HAL_FLASH_Lock();
  return status == HAL_OK;
}

void Configuration::resetToDefaults()
{
  if ( eraseStationDataPage() )
    reportStationData();

  erasePage(CONFIGURATION_FLAG_ADDRESS);
}

bool Configuration::writeStationData(const StationData &data)
{
  if ( !eraseStationDataPage() )
    return false;

  memcpy(&__page.station, &data, sizeof data);
  if ( eraseStationDataPage() )
    {
      bool success = writeStationDataPage();
      reportStationData();
      return success;
    }
  else
    {
      return false;
    }
}

bool Configuration::writeStationDataPage()
{
  uint32_t pageAddress = STATION_DATA_ADDRESS;
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
  HAL_FLASH_Unlock();
  HAL_StatusTypeDef status = HAL_OK;
  for ( uint32_t dw = 0; dw < sizeof __page/8; ++dw )
    {
      status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, pageAddress + dw*8, __page.dw[dw]);
      if ( status != HAL_OK )
        break;
    }
  HAL_FLASH_Lock();

  return status == HAL_OK;
}

bool Configuration::readStationData(StationData &data)
{
  memcpy(&__page, (const void*)STATION_DATA_ADDRESS, sizeof __page);
  memcpy(&data, &__page.station, sizeof data);
  return data.magic == STATION_DATA_MAGIC;
}

const ConfigFlags* Configuration::readConfigFlags()
{
  const ConfigPage *res = (const ConfigPage*)CONFIGURATION_FLAG_ADDRESS;
  if ( res->config.magic != CONFIG_FLAGS_MAGIC )
    return nullptr;

  return &res->config;
}

//__attribute__ ((long_call, section (".code_in_ram")))
bool Configuration::writeConfigFlags(const ConfigFlags *flags)
{
  if ( !erasePage(CONFIGURATION_FLAG_ADDRESS) )
    return false;

  ConfigPage page;
  memcpy(&page, flags, sizeof page);


  uint32_t pageAddress = CONFIGURATION_FLAG_ADDRESS;
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
  if ( HAL_FLASH_Unlock() != HAL_OK )
    return false;

  HAL_StatusTypeDef status = HAL_OK;
  for ( uint32_t dw = 0; dw < sizeof page/8; ++dw )
    {
      status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, pageAddress + dw*8, page.dw[dw]);
      if ( status != HAL_OK )
        break;
    }
  HAL_FLASH_Lock();

  return status == HAL_OK;
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




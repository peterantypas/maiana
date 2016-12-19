/*
 * CommandProcessor.cpp
 *
 *  Created on: Jun 24, 2016
 *      Author: peter
 */

#include "CommandProcessor.hpp"
#include <cstring>
#include <cstdio>
#include "globals.h"
#include "stm32f30x.h"
#include "Configuration.hpp"

CommandProcessor &
CommandProcessor::instance()
{
    static CommandProcessor __instance;
    return __instance;
}

CommandProcessor::CommandProcessor()
{

}

void
CommandProcessor::init()
{
    EventQueue::instance ().addObserver (this, REQUEST_EVENT | RESET_EVENT);
}

void
CommandProcessor::processEvent(const Event &e)
{
    if (e.type == RESET_EVENT) {
        NVIC_SystemReset ();
        return;
    }

    switch (e.request.operation) {
        case OP_GET:
            if (strcmp (e.request.field, "mmsi") == 0)
                returnMMSI ();
            else if (strcmp (e.request.field, "name") == 0)
                returnName ();
            else if (strcmp (e.request.field, "callsign") == 0)
                returnCallSign ();
            else if (strcmp (e.request.field, "beam") == 0)
                returnBeam ();
            else if (strcmp (e.request.field, "length") == 0)
                returnLength ();
            else if (strcmp (e.request.field, "mode") == 0)
                returnMode ();
            else if (strcmp (e.request.field, "vesseldata") == 0)
                returnVesselData ();
            else if (strcmp (e.request.field, "version") == 0)
                returnVersion ();
            else if (strcmp (e.request.field, "status") == 0)
                returnStatus ();
            else
                sendError ("Unknown field");
            break;
        case OP_SET:
            if (strcmp (e.request.field, "mmsi") == 0)
                setMMSI (atoi (e.request.value));
            else if (strcmp (e.request.field, "name") == 0)
                setName (e.request.value);
            else if (strcmp (e.request.field, "callsign") == 0)
                setCallSign (e.request.value);
            else if (strcmp (e.request.field, "beam") == 0)
                setBeam (atoi (e.request.value));
            else if (strcmp (e.request.field, "length") == 0)
                setLength (atoi (e.request.value));
            else if (strcmp (e.request.field, "mode") == 0)
                setMode (e.request.value);
            else if (strcmp (e.request.field, "vesseldata") == 0)
                setVesselData (e.request.value);
            else
                sendError ("Unknown field");
            break;

    }
}

void
CommandProcessor::sendError(const char *err)
{
    Event *reply = EventPool::instance ().newEvent (RESPONSE_EVENT);
    reply->response.success = false;
    strcpy (reply->response.data, err);
    EventQueue::instance ().push (reply);
}

void
CommandProcessor::sendEmptyReply(bool success)
{
    Event *reply = EventPool::instance ().newEvent (RESPONSE_EVENT);
    reply->response.success = success;
    reply->response.data[0] = 0;
    EventQueue::instance ().push (reply);
}

void
CommandProcessor::returnVersion()
{
    Event *reply = EventPool::instance ().newEvent (RESPONSE_EVENT);
    reply->response.success = true;
    strcpy (reply->response.data, REVISION);
    EventQueue::instance ().push (reply);
}

void
CommandProcessor::returnMMSI()
{
    StationData data;
    Configuration::instance ().readStationData (data);

    Event *reply = EventPool::instance ().newEvent (RESPONSE_EVENT);
    reply->response.success = true;
    sprintf (reply->response.data, "%lu", data.mmsi);
    EventQueue::instance ().push (reply);
}

void
CommandProcessor::returnCallSign()
{
    StationData data;
    Configuration::instance ().readStationData (data);

    Event *reply = EventPool::instance ().newEvent (RESPONSE_EVENT);
    reply->response.success = true;
    sprintf (reply->response.data, "%s", data.callsign);
    EventQueue::instance ().push (reply);
}

void
CommandProcessor::returnName()
{
    StationData data;
    Configuration::instance ().readStationData (data);

    Event *reply = EventPool::instance ().newEvent (RESPONSE_EVENT);
    reply->response.success = true;
    sprintf (reply->response.data, "%s", data.name);
    EventQueue::instance ().push (reply);
}

void
CommandProcessor::returnBeam()
{
    StationData data;
    Configuration::instance ().readStationData (data);

    Event *reply = EventPool::instance ().newEvent (RESPONSE_EVENT);
    reply->response.success = true;
    sprintf (reply->response.data, "%d", data.beam);
    EventQueue::instance ().push (reply);
}

void
CommandProcessor::returnLength()
{
    StationData data;
    Configuration::instance ().readStationData (data);

    Event *reply = EventPool::instance ().newEvent (RESPONSE_EVENT);
    reply->response.success = true;
    sprintf (reply->response.data, "%d", data.len);
    EventQueue::instance ().push (reply);
}

void
CommandProcessor::returnMode()
{
}

void
CommandProcessor::returnVesselData()
{
    StationData data;
    Configuration::instance ().readStationData (data);

    Event *reply = EventPool::instance ().newEvent (RESPONSE_EVENT);
    reply->response.success = true;

    sprintf (reply->response.data, "%lu,%s,%s,%d,%d", data.mmsi, data.name,
             data.callsign, data.beam, data.len);
    EventQueue::instance ().push (reply);
}

void
CommandProcessor::returnStatus()
{
}

void
CommandProcessor::setMMSI(uint32_t mmsi)
{
    StationData data;
    if ( Configuration::instance ().readStationData (data) ) {
        data.mmsi = mmsi;
    }
    else {
        memset(&data, 0, sizeof data);
        data.magic = STATION_DATA_MAGIC;
        data.mmsi = mmsi;
    }
    Configuration::instance ().writeStationData (data);
    sendEmptyReply (true);
}

void
CommandProcessor::setName(const char* name)
{
    StationData data;
    if ( Configuration::instance ().readStationData (data) ) {
        strncpy (data.name, name, sizeof data.name);
    }
    else {
        memset(&data, 0, sizeof data);
        data.magic = STATION_DATA_MAGIC;
        strncpy (data.name, name, sizeof data.name);
    }
    Configuration::instance ().writeStationData (data);
    sendEmptyReply (true);
}

void
CommandProcessor::setCallSign(const char *callsign)
{
    StationData data;
    if ( Configuration::instance ().readStationData (data) ) {
        strncpy (data.callsign, callsign, sizeof data.callsign);
    }
    else {
        memset(&data, 0, sizeof data);
        data.magic = STATION_DATA_MAGIC;
        strncpy (data.callsign, callsign, sizeof data.callsign);
    }
    Configuration::instance ().writeStationData (data);
    sendEmptyReply (true);
}

void
CommandProcessor::setBeam(uint8_t beam)
{
    StationData data;
    if ( Configuration::instance ().readStationData (data) ) {
        data.beam = beam;
    }
    else {
        memset(&data, 0, sizeof data);
        data.magic = STATION_DATA_MAGIC;
        data.beam = beam;
    }
    Configuration::instance ().writeStationData (data);
    sendEmptyReply (true);
}

void
CommandProcessor::setLength(uint8_t len)
{
    StationData data;
    if ( Configuration::instance ().readStationData (data) ) {
        data.len = len;
    }
    else {
        memset(&data, 0, sizeof data);
        data.magic = STATION_DATA_MAGIC;
        data.len = len;
    }
    Configuration::instance ().writeStationData (data);
    sendEmptyReply (true);
}

void
CommandProcessor::setVesselData(const char *s)
{
    StationData data;
    if ( !Configuration::instance ().readStationData (data) ) {
        memset(&data, 0, sizeof data);
        data.magic = STATION_DATA_MAGIC;
    }


    /*
     * TODO: Add to/from CSV methods in StationData and use tokenization instead of sscanf()
     */
    int items = sscanf (s, "%lu,%s,%s,%cu,%cu", &data.mmsi, data.name, data.callsign, &data.beam, &data.len);
    if (items < 5)
        sendEmptyReply (false);
    else {
        Configuration::instance ().writeStationData (data);
        sendEmptyReply (true);
    }
}

void
CommandProcessor::setMode(const char *mode)
{
    if (strcmp (mode, "dfu") == 0) {
        FLASH_Unlock ();
        FLASH_Status status = FLASH_WaitForLastOperation (FLASH_ER_PRG_TIMEOUT);
        if (status != FLASH_COMPLETE) {
            sendError ("Unable to unlock flash");
            return;
        }

        FLASH_ErasePage (FIRMWARE_METADATA_ADDRESS);
        status = FLASH_WaitForLastOperation (FLASH_ER_PRG_TIMEOUT);
        if (status != FLASH_COMPLETE) {
            sendError ("Unable to erase metadata page");
            return;
        }

        FLASH_Lock ();
        FLASH_WaitForLastOperation (FLASH_ER_PRG_TIMEOUT);
        if (status != FLASH_COMPLETE) {
            sendError ("Unable to lock flash");
            return;
        }

        /*
         *  UART takes a while to send reply, so unless we add considerable delay here, it won't make it before MCU reset
         */
        //sendEmptyReply (true);
        Event *e = EventPool::instance ().newEvent (RESET_EVENT);
        EventQueue::instance ().push (e);
    }
    else if (strcmp (mode, "rx") == 0) {
        // If there is no valid station data, this command is a NOOP
        StationData data;
        if ( Configuration::instance ().readStationData (data) ) {
            data.flags |= STATION_RX_ONLY;
            Configuration::instance ().writeStationData (data);
        }
        sendEmptyReply (true);
    }
    else if (strcmp (mode, "trx") == 0) {
        // If there is no valid station data, this command is a NOOP
        StationData data;
        if ( Configuration::instance ().readStationData (data) ) {
            data.flags &= ~(STATION_RX_ONLY);
            Configuration::instance ().writeStationData (data);
        }
        sendEmptyReply (true);
    }
    else
        sendError ("Unrecognized mode");
}


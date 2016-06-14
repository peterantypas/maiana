/*
 * Terminal.hpp
 *
 *  Created on: Apr 19, 2016
 *      Author: peter
 */

#ifndef DATATERMINAL_HPP_
#define DATATERMINAL_HPP_

#include <inttypes.h>
#include "ApplicationMetadata.h"
#include "md5.h"


typedef enum {
    STATE_WAITING = 0,
    STATE_IN_TRANSFER_HEADER,
    STATE_IN_TRANSFER_BLOCK
}
TerminalState;

#define ACK     (uint8_t)0x00
#define NACK    (uint8_t)0x01

#define START_TRANSFER_CMD      0xd4
#define START_BLOCK_CMD         0xd6


class DataTerminal
{
public:
    static DataTerminal &instance();

    void init();

    void write(const char* line);
    void processByte(uint8_t b);
    void clearScreen();
private:
    DataTerminal();

    void _write(const char* s);
    void writeCmd(uint8_t cmd);
    void fail();
    void flushPage();
    void flushMetadata();
    void unlockFlash();
    void lockFlash();
private:
    TerminalState mState;
    uint32_t mByteCount;
    ApplicationMetadata mMetadata;
    MD5_CTX mMD5;
    char mCurrPage[FLASH_PAGE_SIZE];
    uint32_t mWriteAddress;
};



#endif /* DATATERMINAL_HPP_ */

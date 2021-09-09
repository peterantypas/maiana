/*
NMEA2000_mcp.h

Copyright (c) 2015-2020 Timo Lappalainen, Kave Oy, www.kave.fi

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Inherited NMEA2000 object for Arduino CAN_BUS shield or any MCP2515 CAN controller
based setup. See also NMEA2000 library.
*/

#ifndef _NMEA2000_MCP_H_
#define _NMEA2000_MCP_H_

// CAN_BUS_shield libraries will be originally found on https://github.com/Seeed-Studio/CAN_BUS_Shield
// That does not work completely with N2k or with Maple mini. So there is developed
// branch found on https://github.com/ttlappalainen/CAN_BUS_Shield
#include "mcp_can.h"
#include "NMEA2000.h"
#include "N2kMsg.h"

// Define size of
#ifndef MCP_CAN_RX_BUFFER_SIZE
#define MCP_CAN_RX_BUFFER_SIZE 50
#endif

class tNMEA2000_mcp : public tNMEA2000
{
private:
  MCP_CAN N2kCAN;
  unsigned char N2k_CAN_CS_pin;
  unsigned char N2k_CAN_clockset;
  unsigned char N2k_CAN_int_pin;
  bool IsOpen;

protected:
  struct tCANFrame {
    unsigned long id; // can identifier
    uint8_t len; // length of data
    uint8_t buf[8];
  };

  class tFrameBuffer {
  protected:
    volatile size_t head;
    volatile size_t tail;
    volatile size_t count;
    volatile size_t size;
    volatile tCANFrame *buffer;
  public:
    tFrameBuffer(size_t _size) : head(0), tail(0), count(0), size(_size) { if ( size<2 ) size=2; buffer=new tCANFrame[size]; }

    void IncWrite() volatile {
      if ( count==size ) return;
      head = (head + 1) % size;
      count++;
    }
    
    void DecRead() volatile {
      if ( count==0 ) return;
      tail = (tail + 1) % size;
      count--;
    }
    
    volatile tCANFrame *GetWriteFrame() volatile {
      if ( count==size ) return 0;
      
      return &(buffer[head]);
    }

    volatile tCANFrame *GetReadFrame() volatile {
      if ( count==0 ) return 0;
      
      return &(buffer[tail]);
    }

    bool AddFrame(unsigned long id, unsigned char len, const unsigned char *buf) volatile {

      if ( count==size || len>8 ) return false;

      buffer[head].id=id;
      buffer[head].len=len;
      len=(len<8?len:8);
      for (uint8_t i=0; i<len; buffer[head].buf[i]=buf[i], i++);
      IncWrite();

      return true;
    }
    
    bool GetFrame(unsigned long &id, unsigned char &len, unsigned char *buf) volatile {
      if ( IsEmpty() ) return false;

      id = buffer[tail].id;
      len = buffer[tail].len;
      for (uint8_t i=0; i<len; buf[i]=buffer[tail].buf[i], i++);
      DecRead();
      return ( (id!=0) && (len!=0) );
    }

    bool IsEmpty() volatile { return (count == 0); }
    
    void Clear() volatile { count=0; head=0; tail=0; }
  };

#if defined(DEBUG_NMEA2000_ISR)
protected:
  volatile unsigned long ISRElapsed;
  virtual void TestISR();
#endif
  
  protected:
  volatile tFrameBuffer *pRxBuffer;
  volatile tFrameBuffer *pTxBuffer;
  volatile tFrameBuffer *pTxBufferFastPacket;

protected:
    bool CANSendFrame(unsigned long id, unsigned char len, const unsigned char *buf, bool wait_sent=true);
    bool CANOpen();
    bool CANGetFrame(unsigned long &id, unsigned char &len, unsigned char *buf);
    bool UseInterrupt() {
      //return N2k_CAN_int_pin!=0xff;
      return false;
    }
    virtual void InitCANFrameBuffers();

public:
    /*
    tNMEA2000_mcp(unsigned char _N2k_CAN_CS_pin, unsigned char _N2k_CAN_clockset = MCP_16MHz,
                  unsigned char _N2k_CAN_int_pin = 0xff, uint16_t _rx_frame_buf_size=MCP_CAN_RX_BUFFER_SIZE);
                  */
    //void SetSPI(SPIClass *_pSPI) { N2kCAN.setSPI(_pSPI); }

    tNMEA2000_mcp(unsigned char _N2k_CAN_clockset = MCP_16MHz,
                  uint16_t _rx_frame_buf_size=MCP_CAN_RX_BUFFER_SIZE);
    void InterruptHandler();
};

#endif

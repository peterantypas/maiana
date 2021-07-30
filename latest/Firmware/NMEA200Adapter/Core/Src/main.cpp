#include <bsp/bsp.hpp>
#include <NMEA2000_mcp.h>
#include "EventQueue.hpp"
#include "NMEA0183Consumer.hpp"
#include "Config.h"

#if DEBUG_LISTENER
#include "DebugMsgHandler.hpp"
#endif

int main(void)
{
  bsp_init();
#if 1
  printf("MAIANA NMEA2000 adapter started\r\n");

  EventPool::instance().init();
  EventQueue::instance().init();

  tNMEA2000 *n2k = new tNMEA2000_mcp();

#if DEBUG_LISTENER
  n2k->SetMode(tNMEA2000::N2km_ListenOnly);
  n2k->SetN2kCANReceiveFrameBufSize(256);

  DebugMsgHandler handler;
  n2k->AttachMsgHandler(&handler);
#else
  // TODO: BSP abstraction for address persistence to/from flash
  n2k->SetMode(tNMEA2000::N2km_NodeOnly, bsp_last_can_address());
  n2k->SetN2kCANSendFrameBufSize(256);
  n2k->ExtendTransmitMessages(TXMessages);
#endif

  n2k->SetProductInformation("",            // Manufacturer's Model serial code
                             1,             // Manufacturer's product code
                             "MAIANA",      // Manufacturer's Model ID
                             "1.0.1",       // Manufacturer's Software version code
                             "1.0",         // Manufacturer's Model version
                             0);

  n2k->SetDeviceInformation(999,        // Serial number -- we don't have one
                            195,        // AIS
                            60,         // Navigation
                            2046,       // Just an unassigned value
                            0);



  if ( n2k->Open() )
    {
      printf("Successfully opened CAN bus\r\n");
    }
  else
    {
      printf("Failed to open CAN bus!!\r\n");
    }

#if !DEBUG_LISTENER
  NMEA0183Consumer::instance().init(n2k);
#endif

  while (1)
    {
      // Our own event loop
      EventQueue::instance().dispatch();

      // This is the NMEA2000 processing loop
      n2k->ParseMessages();

      // We have Systick at a minimum, so this is 1ms at most
      __WFI();

      // Only run this about every 10 seconds
      static uint32_t c = 1;
      if ( c % 10000 == 0 && n2k->ReadResetAddressChanged() )
        {
          bsp_save_can_address(n2k->GetN2kSource());
          c = 1;
        }

      ++c;
    }
#else
  printf("Hello world, %lu\r\n", millis());
  //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
  while(1)
    {
      //__WFI();
      bsp_delay_us(25000);
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
    }
#endif

  return 1;
}


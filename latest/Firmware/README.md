## Transponder firmware

This is a vanilla Eclipse CDT project. Import it into your workspace and (assuming you have ARM cross compiler installed) it should be straightforward.

The application is mainly interrupt driven, with a single event queue being dispatched by an RTOS task. 

### Key updates

 - Board 6.1 bringup is complete
 - DFU can be achieved by jumping directly into the ROM bootloader, so there is no need for hardware to manipulate the BOOT0 pin
 - The TX_DISABLE signal is now supported
 - There is an option to build this firmware as a FreeRTOS application, but the default is bare metal as I don't see much value in this anymore.
 - A significant overhaul of the bit clock ISRs has greatly improved packet reception.
 

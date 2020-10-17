## Transponder firmware

This is a vanilla Eclipse CDT project. Import it into your workspace and (assuming you have ARM cross compiler installed) it should be straightforward.

The application is mainly interrupt driven, with a single event queue being dispatched by an RTOS task. 

### Key updates

 - With board 6.1 bringup complete, the system now runs FreeRTOS 10
 - DFU can be achieved by jumping directly into the ROM bootloader, so there is no need for hardware to manipulate the BOOT0 pin
 - The TX_DISABLE signal is now supported
 

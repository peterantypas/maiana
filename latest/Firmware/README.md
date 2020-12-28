## Transponder firmware

This is a vanilla Eclipse CDT project. Import it into your workspace and (assuming you have ARM cross compiler installed) it should be straightforward.

The application is mainly interrupt driven, with a single event queue being dispatched by either main() or an RTOS task. 

### Key updates

 - It turned out that FreeRTOS added no value, so it was removed
 - DFU can be achieved by jumping directly into the ROM bootloader, so there is no need for hardware to manipulate the BOOT0 pin
 - The TX_DISABLE signal is now supported. It is active low in board 9.x and active high in board 10.x 
 - Board 9.3 adds 3 "status" (LED driving) signals for GPS, RX and TX
 - Board 10.x needed a GPIO swap to make routing viable and inverted the logic of the TX_DISABLE signal, but it's all abstracted in the BSP 
 - The new 'cli' command was introduced to reboot the board into 'quiet' mode, making it a lot easier to program and verify the station data
 - Station metadata is now stored in MCU flash, so it is handled in the Configuration class instead of the BSP. The EEPROM is obsolete.
 

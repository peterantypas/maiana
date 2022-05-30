# MAIANA Bootloader

This is a custom bootloader that installs at flash address 0x08000000. In addition to providing firmware updates, this bootloader
also performs integrity checking and can even rescue a board if a bad application firmware binary is installed.

## Installation

The bootloader must be built and flashed via SWD. An alternative is to use the ST ROM bootloader to install the binary via UART. 
This will require manipulation of the BOOT0 signal though, which is only exposed via a test pad for a bed-of-nails fixture.

## Firmware update

All application firmware must be isntalled via UART. See the example script in the Python subdirectory for the protocol.

## Integrity checking

This bootloader maintains a metadata page at flash address 0x08004000. It holds the size and checksum of the installed application image (if any). 
The application image must be linked for installation at address 0x08004800. If no application is present or the checksum fails, the bootloader enters
DFU mode automatically.

## Rescue mode

If the TX switch is toggled twice during the first 2.5 seconds after power-up, the bootloader enters DFU mode again, regardless of application firmware state.



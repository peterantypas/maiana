### Bootloader script
This Python script interacts with the ST ROM bootloader to erase and write a new firmware image.
To use it, you will first need to place the unit into DFU mode by sending the "dfu" command from the CLI. Don't forget to disconnect your serial terminal first.

Alternatively, the script could be modified to send the "dfu" command, then re-connect with even parity enabled and engage the bootloader.


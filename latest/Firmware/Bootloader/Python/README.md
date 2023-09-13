# MAIANA bootloader Python scripts

To install a binary after flashing the bootloader, run one of the included Python scripts. The arguments are:

- The serial port name (platform dependent)
- The path to the application binary

## Rescuing the board

If you end up with an unusable system because you installed the wrong application binary, power cycle it and toggle the "TX" switch quickly during the first 2 seconds. This will force the bootloader to enter DFU mode so you can try installing the correct binary.




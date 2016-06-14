# Bootloader
The bootloader is a small (~10K) application that occupies the first few pages of flash at 0x0800000.
Its only purpose is to determine whether an application is installed by inspecting a certain flash page. 
If the application is installed it just passes control to it, otherwise it enters UART software upload mode.

When an upload completes, the bootloader writes the special metadata page and resets the system, which will 
cause its next reincarnation to launch the application.

When the application receives a command to start a "software update", it will simply erase the special metadata
and reset the system, which will cause the bootloader to enter UART upload mode.



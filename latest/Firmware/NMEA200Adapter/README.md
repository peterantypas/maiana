# maiana-n2k
This is the firmware for MAIANA's NMEA 2000 breakout adapter.

It currently runs on an STM32L422 and relies on a MCP2515 CAN controller with a TCAN33 bus driver.

It incorporates a copy of [Timo Lappalainen's NMEA 2000 library](https://github.com/ttlappalainen/NMEA2000),  with a few modifications I found necessary. My main addition is PGN 129041 (AtoN report) which I was able to reverse engineer. I'm hoping to merge this back into Timo's library, but the rest of the changes are very much STM32-HAL specific so they probably won't be incorporated.

In addition, it incorporates a copy of the [modified CAN BUS Shield library](https://github.com/ttlappalainen/CAN_BUS_Shield), which I again had to modify slightly to be able to migrate away from Arduino.

Another library that is embedded here is [libais](https://github.com/schwehr/libais) which I had to modify to disable exceptions. This library is very good at parsing NMEA0183 messages, but it was really designed for a microprocessor rather than a microcontroller. Nevertheless, the STM32L422 seems to have enough RAM for it.

I am making no claims that this code base is clean and tidy ;P I can only attest to the fact that it works with at least 2 NMEA 2000 adapters: Digital Yacht iKonvert and Actisense NGW-1.


### Building
You will need Eclipse CDT with GNU ARM GCC. It's possible that the project configuration will not suit your own environment and you'll have to tweak settings. I never quite managed the art of Eclipse project sharing!

### Installing
The adapter must be placed into DFU mode using a pushbutton. This switches the USB serial adapter multiplexer to the MCU's USART1 so you can use ST's bootloader programmer for flashing the binary. There is no SWD interface exposed on that board. Also, the board is powered exclusively from the NMEA2000 bus so it must be plugged in.

### License
MIT, so it's consistent with Timo's license.





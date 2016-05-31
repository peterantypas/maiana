# Open Source AIS Transponder (Class B)

This is the first ever (that I'm aware of) attempt at creating an open source AIS transponder. The prototype device
based on this code and designs is fully functional, but there is still a long way to go before it is a "compliant" unit.

I intend to publish a blog (linking to and from here) as the project evolves. This repository will host the latest Eagle CAD files as well as
the source code for the microcontroller. The source code project requires Eclipse CDT, but I'm open to transitioning to something
else if there is enough interest and participation.


## Overall architecture

On the hardware side, the design is based on two Silicon Labs 4463 transceiver ICs and an STM32F302CBT6 ARM Cortex M4 microcontroller.
The GPS is a GlobalTop "LadyBird" unit, but any decent GPS module with NMEA and PPS output should work.
The receiver incorporates a bandpass / LNA (NXP BGA2869) and a Skyworks 66100 front end (PA/switch).
The transmitter output is nominally 0.5Watts (+27dBm) and it has a verified range of 5 nautical miles with a vanilla telescopic antenna (< 3dBi).
The circuit is powered entirely from a USB connection which also delivers NMEA (GPS + AIS) data to the host. It draws 135 mA in RX mode,
and spikes up to 350 mA during transmission at full power. For persisting station data there is a 1Kbit Microchip EEPROM on board.

The microcontroller communicates with the two RF ICs using the same SPI bus (SPI1), so SPI operations cannot overlap.
That's why after initialization and interrupt configuration, *all SPI transactions occur in interrupt mode*. The EEPROM
is attached to I2C1. Remarkably, it worked fine with the MCU's internal pull-ups, but I updated
the design to include external pull-up resistors on the SDA and SCL lines anyway. The code should be modified if you choose to 
install those.


On the software side, the microcontroller runs in both thread and interrupt mode. After hardware initialization,
*main()* dispatches events while keeping the watchdog happy. Interrupt code performs as little work as possible 
and queues up events for non-real time operations to be processed in thread mode. 

Memory is managed almost entirely using pre-allocated pools of objects (RX and TX packets, NMEA sentences, etc). 

Instead of using ST Micro's semihosting for debug output, I implemented *printf2()* (see printf2.cpp) which writes to USART2 at 
a very high speed (230400 bps). This allowed me to identify issues when running "release" code with no debugger attached,
which as you might suspect behaves quite differently with respect to timing.

Lastly, I opted for the Standard Peripheral Library instead of HAL/CubeMX because it is very stable. I would consider migrating to libopencm3 or something similar to open up *more* microcontroller options.

 




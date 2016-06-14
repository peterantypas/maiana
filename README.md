# Open Source AIS Transponder (Class B)

This is the first ever (that I'm aware of) attempt at creating an open source AIS transponder. The prototype device
based on this code and designs is fully functional, but there is still a long way to go before it is a "compliant" unit.

I intend to publish a blog (linking to and from here) as the project evolves. This repository will host the latest Eagle CAD files as well as
the source code for the microcontroller. The source code project requires Eclipse CDT, but I'm open to transitioning to something
else if there is enough interest and participation.


## Overall description

### Hardware

On the hardware side, the design is based on two Silicon Labs 4463 transceiver ICs and an STM32F302CBT6 ARM Cortex M4 microcontroller.
The GPS is a GlobalTop "LadyBird" unit, but any decent GPS module with NMEA and PPS output should work.
The radio incorporates an external bandpass / LNA (NXP BGA2869) and a Skyworks 66100 front end (PA/switch).
The transmitter output is nominally 0.5Watts (+27dBm) and it has a verified range of 5 nautical miles with a vanilla telescopic antenna (< 3dBi).
The circuit is powered entirely from a 5V connection (USB for now, but leaning against it long term). It draws 135 mA in RX mode,
and spikes up to 350 mA during transmission at full power. For persisting station data there is a 1Kbit Microchip EEPROM on board.
I intend to use a Raspberry Pi as the front end of the transceiver, as the unit is supposed to be mounted outside, directly connected to its own antenna.
The Pi will act as a source of power, a WiFi Access Point, a NMEA distributor and a web server for configuration and software updates. All communication between the transponder
and the Pi is done over a single serial port.

Persistent station data (MMSI, call sign, name, dimensions, etc) is stored on a 1Kbit EEPROM attached to I2C1. Remarkably, it works fine with the MCU's 
internal pull-ups, but I updated the design to include external pull-up resistors on the SDA and SCL lines. The code should be modified if you choose to 
install those.

### Software

There are two programs that need to be installed on the flash. The bootloader and the main application. The bootloader is
optional, but it allows for software update via a very simple (albeit proprietary) serial protocol.

On the software side, unlike most examples you see online the microcontroller runs in _both_ thread and interrupt mode. After hardware initialization,
*main()* dispatches events while keeping the watchdog happy. Interrupt code performs as little work as possible 
and queues up events for non-real time operations to be processed in thread mode. There is no operating system, and I could never
justify the overhead of one.

Lastly, I opted for the Standard Peripheral Library instead of HAL/CubeMX because I found it to be more stable. The library is included here.
 




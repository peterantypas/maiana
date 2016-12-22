# Open Source AIS Transponder (Class B)

This is the first ever (that I'm aware of) attempt at creating an open source AIS transponder. The prototype device
based on this code and designs is fully functional, but there is still a long way to go before it is a "compliant" unit.

I intend to publish a blog (linking to and from here) as the project evolves. This repository will host the latest Eagle CAD files as well as
the source code for the microcontroller. The source code project requires Eclipse CDT, but I'm open to transitioning to something
else if there is enough interest and participation.


## Overall description

### Hardware

On the hardware side, the design is based on two Silicon Labs 4463 transceiver ICs and an STM32F302CBT6 ARM Cortex M4 microcontroller.
The GPS is a GlobalTop "LadyBird" unit, but any decent GPS module with NMEA and PPS output should work. On the front end, there is an
external LNA (NXP BGA2869) with an LC bandpass filter, a 2 Watt MOSFET PA and an active Skyworks RF switch. 


I intend to use a Raspberry Pi Zero as the front end of the transceiver, as the unit is supposed to be mounted outside, directly connected to its own antenna.
The Pi will act as a WiFi Access Point and/or Bluetooth peripheral, a NMEA distributor and a web server for configuration and software updates.
All communication between the transponder and the Pi will be carried out over a single serial port.


### Software

There are two programs that need to be installed on the Flash. The [bootloader](bootloader/) and the main [application](application/). 







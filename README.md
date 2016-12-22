# Open Source AIS Transponder (Class B)

This is the first ever (that I'm aware of) attempt at creating an open source AIS transponder. The prototype device
based on this code and designs is fully functional, but there is still a long way to go before it is a "compliant" unit.

I intend to publish a blog (linking to and from here) as the project evolves. This repository will host the latest Eagle CAD files as well as
the source code for the microcontroller. The source code project requires Eclipse CDT, but I'm open to transitioning to something
else if there is enough interest and participation.


## Overall description

### Hardware

On the hardware side, the design is based on two Silicon Labs 4463 transceiver ICs and an STM32F302CBT6 ARM Cortex M4 microcontroller.
One of the SiLabs ICs acts as a transceiver, while the other IC works as a receiver only. During reception, each IC tunes to a different
channel. When a transmission is scheduled, the ICs swap channels if necessary for the transceiver to be listening on the next transmit channel.
The swap occurs exactly at the boundary of a SOTDMA time slot, so no messages are lost.
This configuration makes for a much simpler PCB layout and negates the need for a 3-position RF switch.

The GPS is a GlobalTop "LadyBird" unit, but any decent GPS module with NMEA and PPS output should work.

The RF board incorporates an external bandpass / LNA (NXP BGA2869) and a Skyworks 66100 front end (PA/switch).

The new 4-layer PCB incorporates a 2 Watt (+33dBm) PA powered by the very inexpensive NXP AFT0SMS003N MOSFET ($1.60 each at retail).

Persistent station data (MMSI, call sign, name, dimensions, etc) is supposed to live at a high page in Flash, way beyond the application and bootloader code boundaries.

The circuit requires 7.5 Volts (because of the RF MOSFET).  It draws 100 mA in RX mode, and spikes up to 680 mA during transmission at full power.
Surge current for transmissions can be provided by a 10 mF supercapacitor so that the drain on the power supply can remain constant.

I intend to use a Raspberry Pi as the front end of the transceiver, as the unit is supposed to be mounted outside, directly connected to its own antenna.
The Pi will act as a source of power, a WiFi Access Point, a NMEA distributor and a web server for configuration and software updates. All communication between the transponder
and the Pi is done over a single serial port.


### Software

There are two programs that need to be installed on the Flash. The [bootloader](bootloader/) and the main [application](application/). 







# MAIANA&trade;: The Open Source AIS Transponder

I started this project around 2015 with the intention of experimenting and eventually building something for my own boat.
I was not impressed with commercial AIS class B transponders. They seemed bloated, expensive and some of them
were particularly power-hungry. So I set out to create a lean and mean design.

In 2018 I installed this system on my boat. It is still working fine after 2 years.

![Image](images/UnitExterior.jpg?raw=True "Exterior View")

I have refined the design quite a bit, and it now has an official name, "Maiana", borrowed from an atoll in the Kiribati islands. I do intend to trademark this, in the same way that "Arduino" is a trademark that identifies the original rather than the many clones which inevitably emerged.

## Overall description

### Hardware

The main difference between this design and nearly every commercial transponder is that it's a standalone unit. It contains all of its
radios and antennas and thus only needs a power and data cable to connect to the cabin. The PCBA is only 1" wide so it fits inside
1" schedule 40 PVC pipe, which I used as the antenna base. The GPS receiver and antenna are on the board:

![Image](images/transponder-9.3.jpg?raw=True "PCBA version 9.3")

The complete design is intended for mounting on 1" railing, similar to these antennas here:

![Image](images/Antenna-Example1.jpg?raw=True "Example 1")
![Image](images/Antenna-Example2.jpg?raw=True "Example 2")
![Image](images/Antenna-Example3.jpg?raw=True "Example 3")

In one of those examples, MAIANA&trade; would take the place of the GPS antenna and add AIS functionality.

The design is based on two Silicon Labs 4463 transceiver ICs and an STM32L412 series microcontroller. The 80MHz clock speed
of the MCU allows the SPI bus to operate at exactly 10MHz which is the maximum supported by the Silabs RF ICs. This is crucial, as a transponder is
a hard real-time application that relies on interrupts for precise timing of the transmit function, so SPI latency must be minimized.

The GPS is a Quectel L70R module and relies on a Johanson ceramic SMD antenna. It usually takes a little over a minute to acquire a fix outdoors.
The transmitter output is 2 Watts (+33dBm) and it has a verified range of over 10 nautical miles.

The unit runs on 12V and exposes a 3.3V UART for connecting to the rest of the boat's system. The UART continuously sends GPS and AIS data in NMEA0183 format at 38.4Kbps.
On my boat, it is wired to a control box that converts the UART to USB and feeds it to a RPi Zero W, which acts as a WiFi access point / NMEA distributor for iNavx
running on an iPad:

![Image](images/MAIANA-ControlBox.jpg?raw=True "Control Box")

Of course, there are many different solutions available and every boater has different preferences.

Persistent station data (MMSI, call sign, name, dimensions, etc) is stored on a 1Kbit EEPROM and is provisioned via a command interface.
The unit implements SOTDMA synchronization based on the very acurate 1 PPS signal from the GNSS and the UTC clock, but being a class B, it will not attempt to reserve time slots.
It will just transmit autonomously and independently, based on Clear Channel Assessment, at the schedule permitted for class B devices. If station data is not provisioned, the device
will simply run as a receiver and never transmit.

The system draws about 30mA from 12V in RX mode, and spikes up to 600mA during transmission (for about 30 milliseconds).

The latest design (9.3 pictured above) relies on plain Cat5 cable for power, data, as well as control signals such as "TX OFF" and status LED drivers (RX/TX/GPS). 


### Software

The firmware is an Eclipse CDT project that you should be able to import and build. The code is C++ with a BSP abstraction layer so you need to tweak bsp/bsp.hpp or define one of
the required symbols in the preprocessor to build for different board revisions. It contains snippets of STM32Cube generated code, but is does not follow ST's spaghetti structure.

### Building the unit

This is going to be difficult for all but the most technically advanced. The board features all surface mounted components, with 4 QFNs, a few SOT-363s and tightly spaced 0603 passives. Unless you're skilled with stencils and reflow, you will find it challenging. 

To make this easier I will release a kit on tindie.com. I was hoping to have it ready by the start of the 2021 boating season (March or April), but Silicon Labs is experiencing a severe supply chain disruption right now, so this may not happen until the second half of 2021. The kit will include a 98% finished PCBA as well as the VHF antenna, enclosure and sealing components. The board will be programmed, tested and calibrated, and the antenna will be perfectly matched, but there's one caveat: <b>I cannot legally ship this as a transponder yet</b>. The workaround is to bill it as an AIS+GPS receiver. To turn it into a transponder, you will have to <b>procure and install [inductor L17](https://github.com/peterantypas/ais_transponder/blob/master/latest/CAD/Board-9.3/transponder-9.3.0-schematic.pdf) on the PCBA yourself</b> (see page 5 of the schematic). Without this part, the board is physically incapable of transmitting. Now, to make this extra easy, I will design a large SMD footprint there so you can do it with a soldering iron, a bit of flux and only basic skills. More instructions to follow ;)

### License

CAD and firmware are licensed under GPLV3. I chose the most "copyleft" license possible to discourage commercial entities from ripping this off and then "close-sourcing" it. 
So don't try anything fishy, because I *will* find out and then ... well, let's just say you'll be buying me another boat and I have a particular one in mind ;)

If you're a tinkerer and want to further this design while adhering to the GPL, more power to you! Build it, sell it, give it away, do whatever you want to get it out there. 
You cannot, however, use the MAIANA&trade; name in a forked design, as it will be trademarked. Just call it something else.










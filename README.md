# MAIANA&trade;: The Open Source AIS Transponder

I started this project around 2015 with the intention of experimenting and eventually building something for my own boat.
I was not impressed with commercial AIS class B transponders. They seemed bloated, expensive and some of them
were particularly power-hungry. So I set out to create a lean and mean design.

In 2018 I installed this system on my boat. It is still working fine after 2 years.

![Image](images/UnitExterior.jpg?raw=True "Exterior View")

I have refined the design somewhat, and it now has an "official" name, borrowed from a Pacific atoll in the Kiribati islands. I do intend to trademark this, in the same way that "Arduino" is a trademark that identifies the original rather than the many clones that inevitably emerged.

## Overall description

### Hardware

The main difference between this design and nearly every commercial transponder is that it's a standalone unit. It contains all of its
radios and antennas and thus only needs a power and data cable to connect to the cabin. The PCBA is only 1" wide so it fits inside
1" schedule 40 PVC pipe, which I used as the antenna base. The GNSS receiver and antenna are on the board:

![Image](images/Board4.3.jpg?raw=True "PCBA version 4.3")

The design is based on two Silicon Labs 4463 transceiver ICs and an STM32L4xx series microcontroller (L412 and L432 as of now).
The GNSS is a Telit SE873 (7x7mm module) and relies on a Johanson ceramic SMD antenna. It usually takes about a minute to acquire a fix outdoors.
The transmitter output is 2 Watts (+33dBm) and it has a verified range of over 10 nautical miles.

The unit runs on 12V and exposes a 3.3V UART for connecting to the rest of the boat's system. The UART continuously sends GPS and AIS data in NMEA0183 format at 38.4Kbps.
On my boat, it is wired to a control box that converts the UART to USB and feeds it to a RPi Zero W, which acts as a WiFi access point / NMEA distributor for iNavx
running on an iPad. There are many different apps and solutions available for this and every boater has different preferences.

Persistent station data (MMSI, call sign, name, dimensions, etc) is stored on a 1Kbit EEPROM and is provisioned via a command interface.
The unit implements SOTDMA synchronization based on the very acurate 1 PPS signal from the GNSS and the UTC clock, but being a class B, it will not attempt to reserve time slots.
It will just transmit autonomously and independently, based on Clear Channel Assessment, at the schedule permitted for class B devices. If station data is not provisioned, the device
will simply run as a receiver and never transmit.

The system draws about 40mA from 12V in RX mode, and spikes up to 600mA during transmission (for about 30 milliseconds).

The latest design (not the one pictured above) relies on Ethernet cable for power, data, and control signals such as "TX OFF". I will include a reference design for a
control box that I'm working on but every boat is different, so your mileage will absolutely vary.


### Software

The firmware is an Eclipse CDT project that you should be able to import and build. The code is C++ with a BSP abstraction layer so you need to tweak bsp/bsp.hpp or define one of
the required symbols in the preprocessor to build for different board revisions. It contains snippets of STM32Cube generated code, but is does not follow ST's spaghetti structure.

### Building the unit

This is going to be difficult for all but the most technically advanced.

The board features all surface mounted components, with 5 QFNs, a few SOT-363s and all 0603 passives. Unless you're skilled with stencils and reflow, you will find it challenging. 

I am going to make it available as part of a kit (probably on tindie.com). The kit will include a 95% finished PCBA as well as the VHF antenna, 
enclosure and sealing components (which are NOT open sourced). The board will be programmed and tested, and the antenna will be perfectly matched. The cable and whatever lies on the 
other side of it will be the buyer's responsibility. 

### License

CAD and firmware are licensed under GPLV3. I chose the most "copyleft" license possible to discourage commercial entities from ripping this off and then "close-sourcing" it. 
So don't try anything fishy, because I *will* find out and then ... well, let's just say you'll be buying me another boat and I have a particular one in mind ;)

If you're a tinkerer and want to further this design while adhering to the GPL, more power to you! Build it, sell it, give it away, do whatever you want to get it out there. 
You cannot, however, use the MAIANA&trade; name in a forked design, as it will be trademarked. Just call it something else.

### Concluding thoughts

The commercial AIS industry has not exactly been successful in the low-cost recreational boating market, and with prices like $500-$800 for a unit that has a bunch of external 
dependencies that's hardly a surprise. I hope to change that.









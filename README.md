# MAIANA&trade;: The Open Source AIS Transponder

I started this project around 2015 with the intention of experimenting and eventually building something for my own boat.
I was not impressed with commercial AIS class B transponders. They seemed bloated, expensive and some of them
were particularly power-hungry. Also, they required at least two external RF cables, one for VHF and one for GPS. So I set out to create a lean and mean design. I must say that I'm quite pleased with the results. Here are some examples of MAIANA&trade; installations:

<img src="images/InstallOnSolarPanels.jpg" height="320"/><img src="images/InstallOnRadarDome.jpg" height="320"/><img src="images/InstallOnSternRail.jpg" height="320"/>

The main difference between MAIANA&trade; and every commercial transponder is that it's a standalone unit. It contains all of its
radios and antennas and thus only needs a power and data connection to the cabin. This requires an RJ45 (Ethernet) cable which runs from the outside unit to one of these breakout boxes down below:

<img src="images/usbadapter.jpg" height="420"/><img src="images/nmea0183adapter.jpg" height="420"/><img src="images/nmea2000adapter.jpg" height="420"/>


## Hardware Design
The antenna casing that you see in these photos is a piece of 1" Schedule 40 "furniture" grade PVC pipe. It is simply the most inexpensive UV resistant material available, and it looks great to boot.

The VHF antenna whip is built using an epoxy would filament tube coated with a high grade, US-made irradiated polyolefin ("heat shrink") tubing. The company that makes this tubing (and helped me with a design challenge) also built the landing gear for NASA's Ingenuity helicopter, currently flying on Mars. The bottomline is that unlike typical fiberglass antenna masts you might see around (or have on your boat), this antenna is not going to degrade and "blossom" under continuous sun exposure.


The transponder circuit is inside this case, built on a 24mm wide PCBA. The GPS receiver and antenna are on the board:

![Image](images/transponder-9.3.jpg?raw=True "PCBA version 9.3")

In its final form, the main unit looks like this:

![Image](images/Maiana-FinalAssembly.jpg?raw=True "Complete unit")

As you can see, the complete design is intended for mounting on 1" railing, similar to these antennas here:

![Image](images/Antenna-Example1.jpg?raw=True "Example 1")
![Image](images/Antenna-Example2.jpg?raw=True "Example 2")
![Image](images/Antenna-Example3.jpg?raw=True "Example 3")

In one of those examples, MAIANA&trade; would take the place of the GPS antenna and add AIS functionality.

The core design is based on two Silicon Labs "EZRadio Pro" series ICs. The first IC is a transceiver and the second one is a full-time receiver. For the transceiver, I originally used the Si4463. For the receiver, either the 4463 or the 4362 work. I'm in the process of evaluating alternatives to the Si4463 to see if I can get enough power output with chips that are currently available, namely the 4460 and 4467. See [this discussion](https://github.com/peterantypas/ais_transponder/discussions/24) for more details on that effort.

The MCU is a STM32L4x2 series microcontroller (412 and 432 supported). I chose this series because the 80MHz clock speed allows the SPI bus to operate at exactly 10MHz which is the maximum supported by the Silabs RF ICs. This is crucial, as a transponder is a hard real-time application that relies on interrupts for precise timing of the transmit function, so SPI latency must be minimized.

The GNSS is a Quectel L76 module and relies on a Johansson ceramic chip antenna. It usually takes a minute to acquire a fix outdoors from a cold start.
The transmitter output is 2 Watts (+33dBm) and it has a verified range of over 10 nautical miles.

The unit runs on 12V and exposes a 3.3V UART for connecting to the rest of the boat's system. The UART continuously sends GPS and AIS data in NMEA0183 format at 38.4Kbps. On my boat, it is wired to a box that converts the UART to USB and feeds it to a RPi Zero W, which acts as a WiFi access point / NMEA distributor:

![Image](images/MAIANA-ControlBox.jpg?raw=True "Control Box")

Of course, there are many different solutions available and every boater has different preferences. This interface box is a separate project and it will eventually support NMEA0183 (RS422) as well as NMEA2000 interfaces.

For the circuit to transmit, it must be configured with persistent station data (MMSI, call sign, name, dimensions, etc). This is stored in MCU flash and is provisioned via a command line interface. If station data is not provisioned, the device will simply run as a receiver.

The unit implements SOTDMA synchronization based on the very acurate 1 PPS signal from the GPS and the UTC clock, but being a class B, it will not attempt to reserve time slots. It will just transmit autonomously and independently, based on Clear Channel Assessment, at the schedule permitted for class B devices. 

The system draws about 30mA from 12V in RX mode, and spikes up to 600mA during transmission (for about 30 milliseconds).

The latest design (9.3 pictured above) relies on plain Cat5 cable for power, data, as well as control signals such as "TX OFF" and status LED drivers (RX/TX/GPS). 


### Software

The firmware is an Eclipse CDT project that you should be able to import and build. The code is C++ with a BSP abstraction layer so you need to tweak bsp/bsp.hpp or define one of
the required symbols in the preprocessor to build for different board revisions. It contains snippets of STM32Cube generated code, but is does not follow ST's spaghetti structure.

### Building the unit

This is going to be difficult for all but the most technically advanced. The board features all surface mounted components, with 4 QFNs, a few SOT-363s and tightly spaced 0603 passives. Unless you're skilled with stencils and reflow, you will find it challenging. 

For this reason, I'm working on a kit and I expect the first 10 or so to be available by end of June / beginning July of this year. This has been a struggle, given the state of the supply chain, so I had to make many last-minute design changes and secure enough BOM. If you're interested in the kit, sign up for my mailing list here: https://eepurl.com/hxHP-5



### License

CAD and firmware are licensed under GPLV3. I chose the most "copyleft" license possible to discourage commercial entities from ripping this off and then "close-sourcing" it. 
So don't try anything fishy, because I *will* find out and then ... well, let's just say you'll be buying me another boat and I have a particular one in mind ;)

If you're a tinkerer and want to further this design while adhering to the GPL, more power to you! Build it, sell it, give it away, do whatever you want to get it out there. 
You cannot, however, use the MAIANA&trade; name in a forked design, as it will be trademarked. Just call it something else.










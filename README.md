# Open Source AIS Transponder (Class B)

This is the first ever (that I'm aware of) attempt at creating an open source AIS transponder. The prototype device
based on this code and designs is fully functional, but there is still a long way to go before it is a "compliant" unit.

I intend to publish a blog (linking to and from here) as the project evolves.

## Overall architecture

On the hardware side, the design is based on two Silicon Labs 4463 transceiver ICs and an STM32F302 ARM Cortex M4 microcontroller.
The GPS is a GlobalTop "LadyBird" unit, but any decent GPS module with NMEA and PPS output would work.

On the analog RF side, the receiver incorporates an active bandpass filter with an NXP BGA2869 LNA, and a SkyWorks 66100 Front End module (PA/switch).
The transmitter output is nominally 0.5Watts (+27dBm) and it has a verified range of 5 nautical miles.

The system is powered entirely from a USB connection which also delivers NMEA (GPS + AIS) data to the host.

### The nitty gritty
Physically, the unit is designed as two overlaid PCBs, joined together with standard 0.01" male/female pin headers, just like
Raspberry Pi and Arduino hats. One PCB holds the logic and GPS stage and the other PCB hosts the RF stage.




## Board design files

This directory contains Eagle CAD files as well as schematic PDFs and Gerber exports for board fabrication.

The board stackup is good enough for either OSH Park 4-layer, or JLCPCB 4-layer with controlled impedance (JLC2313). 


### Latest Board

Revision 11.0.0 does away with the GreenPAK entirely. When firmware needs to turn the GNSS off, it simply cuts off its main power while maintaining power to its RTC. This is not meant to be used under normal operation, but the feature is necessary to support firmware updates to the main unit.

Revision 10.9.0 is a BOM-count optimized design that introduces a custom GreenPAK IC for buffering the GNSS signals, as well as a new
antenna switch which is even smaller than the previous one.

Revision 10.5.0 is the first board to ship as part of the MAIANA kit.

Revision 10.0.1. adds a few safety features such as ESD protection and isolation between the board's MCU pins and the outside world.
An appropriate breakout board for series 10.x is included. The "TX OFF" signal is now buffered by a MOSFET on the transponder board and the
logic is inverted.

The LED "status" signals are now current-limited open drain outputs via N-Channel MOSFETs on the transponder board. This means they could work
with any voltage up to 16V without risk of damage to the MCU.

### Breakouts
These directories contain Eagle and FreeCAD files for the NMEA0183, NMEA 2000 and USB adapters. Their front panels are PCBs so gerbers are
here as well.


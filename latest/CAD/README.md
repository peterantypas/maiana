## Board design files

This directory contains Eagle CAD files as well as schematic PDFs and Gerber exports for board fabrication.

The board stackup is good enough for either OSH Park 4-layer, or JLCPCB 4-layer with controlled impedance (JLC2313). 


### Latest Board

Revision 11.3.0 restores the EEPROM in order to support runtime persistence with no hiccups (STM32s cannot safely erase/write flash in interrupt-heavy applications). This is necessary to support integration with OpenPlotter.

### Breakouts
These directories contain Eagle and FreeCAD files for the NMEA0183, NMEA 2000 and USB adapters. Their front panels are PCBs so gerbers are
here as well. The UART adapter is a very basic breakout board for those who would rather wire the transponder themselves.

### Antenna
This directory contains FreeCAD and STEP exports of the 3D printed parts that make up the antenna.

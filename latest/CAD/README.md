## Board design files

This directory contains Eagle CAD files as well as schematic PDFs and Gerber exports for board fabrication.

The board stackup is good enough for either OSH Park 4-layer, or JLCPCB 4-layer with controlled impedance (JLC2313). 

### Latest Board

Revision 9.3.0 is latest. I built it at JLCPCB with the exact gerber files I posted here. It is fully functional and performed as expected.

The 9.3 directory also contains a reference design for a breakout to help with development and debugging. The breakout
features a CH330N USB-to-UART adapter IC as well as a TX "on/off" switch so you can get some ideas about how to interface to the
unit from your own system. The Gerbers for the breakout are here as well.

The 9.3 board adds 3 "status" signals that can drive LEDs (with appropriate current limiting resistors!). These are optional.
Instead of RJ45 and Cat 5 cable, you can use a 4-wire 0.1" pitch Phoenix type terminal mounted on pads 1,3,5 and 7 of the RJ45
footprint and use a 4-wire cable instead. This will carry power and UART Rx/Tx only.



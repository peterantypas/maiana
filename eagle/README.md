# Eagle CAD files

The new design is a single, 4-layer PCB which does not follow the conventional 4-Layer design paradigm.

- Layer 1 is the component and main signal layer as you might expect.
- Layer 2 is a power and signal layer with no ground pour. The only requirement is that these signals don't cross the RF path anywhere.
- Layer 3 is the ground layer. It is 100% uninterrupted save for via holes.
- Layer 4 is yet another signal layer with ground pour. Because it lies underneath the ground its traces can go anywhere with no consequences.

By increasing the distance between the signal and ground layer, I was able to use very wide coplanar waveguide traces so as to minimize losses and parasitic inductance.
SWR is better than 1.1:1 with a 50 Ohm load.

The board now requires 7.5 Volts (because of the RF MOSFET). It draws 100 mA while receiving, and spikes to 680 mA during transmission at full power.
Surge current for transmission can be provided by a low-profile 10 mF supercapacitor on the back of the PCB. Also, during transmission,
the external LNA is now powered down as a precaution.

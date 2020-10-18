# Board 7.0

This board is not yet completely validated, but its individual components are already proven.

Main improvements:

- The RX path features a MiniCircuits PSA4-5043+ LNA and a SAW filter from RFMI
- The DC/DC switch is now a synchronous buck with spread spectrum support for reduced EMI
- The 10000uF supercapacitor is back as an option in case transmission currents cause too much voltage drop at the input
- There are explicit signals to control status LEDs (GNSS, RX, TX)

There are two assembly variants, with and without the supercap. Firmware will support either without any special considerations, as it enforces a 5 second minimum interval between consecutive transmissions, which is more than enough to recharge a depleted supercap.

The control circuit is designed to fit inside a Hammond 1551KFLGY case (with appropriate cutouts). It features a single on/off button as well as a "TX on/off" slider switch. Control logic is implemented in a customized GreenPAK mixed signal IC (SLG46537V):

![Image](ais-control-greenpak-design.png?raw=True "Exterior View")

Using this type of IC avoids yet another microcontroller with firmware. The GreenPAK instruments latched overcurrent protection, debounces the on/off button, and acts as an LED driver. The design file for the IC (.gp5) is included. If you are an electronics enthusiast and you're not familiar with this family of ICs yet, I think it's time to change that ;)


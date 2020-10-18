# Board 7.0

This board is not yet completely validated, but its individual components are already proven.

Main improvements:

- The RX path features a MiniCircuits PSA4-5043+ LNA and a SAW filter
- The DC/DC switch is now a synchronous buck with spread spectrum for reduced EMI
- The 10000uF supercapacitor is back as an option in case transmission currents cause too much voltage drop at the input
- There are explicit signals to control status LEDs (GNSS, RX, TX)

The control circuit is designed to fit inside a Hammond 1551KFLGY case (with appropriate cutouts). It features a single on/off button as well as a "TX on/off" slider switch.

All logic is provided by a custom Dialog GreenPAK mixed signal IC (SLG46537V):

![Image](ais-control-greenpak-design.png?raw=True "Exterior View")

Using this type of IC avoids yet another microcontroller and associated firmware. Also, the GreenPAK instruments latched overcurrent protection, debouncing of the on/off button, and acts as an LED driver.

The design file for the GreenPAK (.gp5) is included.



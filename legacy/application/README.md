# Main application

The main transponder application initializes the RF ICs and handles all interrupts from the GPS UART and the RF IC clock/data lines.
It also utilizes a few timers, one of which represents SOTDMA time slots and is continuously aligned with the PPS signal from the GPS.

Unlike many examples we see online, the microcontroller runs in _both_ thread and interrupt mode. After hardware initialization,
*main()* enters an infinite loop dispatching events and keeping the watchdog timer happy. Interrupt code (which is sprinkled throughout) 
performs as little work as possible and queues up events for non-real time operations to be processed in thread mode. There is no operating system (I could never
justify the overhead) so this is a very lean approximation of one.

Almost all memory is managed using pre-allocated pools of objects.

The Standard Peripheral Library for stm32f3xx is included.

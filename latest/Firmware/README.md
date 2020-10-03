### Transponder firmware

This is a vanilla Eclipse CDT project. Import it into your workspace and (assuming you have ARM cross compiler installed) it should be straightforward.

The application is mainly interrupt driven, with a single event queue being dispatched by main, so it's pretty much "bare metal". Given how much of the functionality relies on I/O and timer interrupts, the only thing an RTOS would add is a single task to dispatch asynchronous events, which main() can do just fine.
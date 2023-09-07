# Fuzix For Rhyophyre

This is an initial cut at a Fuzix image for the Rhyophyre.

## Hardware

The core system is fairly generic. It's a Z180, 1MB of RAM, some ROM and the
usual Retrobrew PPIDE and RTC interfaces. There's a simple control latch and
a VT82C42 PS/2 port as well.

The big part of the system is the NEC7220 graphics. This is not yet supported
at all as it's a major new piece of work whilst the rest is basically a paste
together of existing supported devices.

The current gen1 board does not support SPI via the CSIO which means there is
no ability to add an SD card or to support TCP/IP via the WizNET devices. You
could in theory do this by stealing some of the spare or borrowable I/O pins
(eg the flow control for the second serial).

## Things To Do

- Support the NEC7220 consoles and PS/2 port
- Look at adjusting the various resources to match memory

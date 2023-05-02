# Fuzix For RC2014 Paged ROM

## Supported Hardware

- Z80 CPU Card
- Paged ROM card configured to map 32K (recommended 28C256 so you can reflash it
  easily)
- IDE CF adapter
- ACIA or SIO card (the original 'wide decode' ACIA is supported)

## Options

- DS1302 Real time clock at 0xC0
- Z80 CTC with channel 2 feeding channel 3

One of the two is required in order to provide a source of time. The CTC provides
proper interrupt driven task switching, the RTC does not and slows the machine
a chunk.

This is btw not a good way to run Fuzix, it's more for the challenge!

## Installation

make diskimage produces two files in the Images/rc2014-tiny directory.
The fuzix.rom file is the replacement ROM image, the disk.img file is a raw
CF card image suitable for writing to the CF adapter


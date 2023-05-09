# Z80-MBC 2

This is a simple system with a Z80, 128K of RAM and an Atmega acting as
the I/O subsystem.

## Supported Hardware

Z80-MBC2 with RTC and SD card

Firmware S220718-R120519 or later is required

## Hardware

https://hackaday.io/project/159973-z80-mbc2-4ics-homemade-z80-computer

##  Install

make diskimage

For emulation then
z80-mbc2 -i -b fuzix.bin

For a real system put the loader as FUZIX.HEX on the D0 FAT partition,
put the built disk image as another volume (eg 1)


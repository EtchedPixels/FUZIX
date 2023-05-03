# SmallZ80

## Bugs

## To Add

Floppy interface
Set up vectors once at boot only (they are not switched)

## Memory Map

This platform has 32K banked and 32K fixed, but rather oddly the banked area
is from 0x4000-0xBFFF, with fixed memory above and below. Also it is unusual
in that the fixed area is a separate RAM not taken from banked space.

Memory Layout

Kernel
0000-00FF	Interrupt vectors
0100-3FFF	Kernel data and some code
4000-BFFF	Kernel code only
C000-FFFF	Kernel code, data and common

User
0000-00FF	Interrupt vectors
0100-3FFF	Kernel data and some code
4000-BDFF	User space
BE00-BFFF	User task state save area
C000-FFFF	kernel code, data and common

## Notes

The only timer source and interrupt source is the RTC. Fortunately this can
provide a 64Hz clock.

The ROM requires the disk can handle a particular layout for CP/M. Not
clear how much it matters for Fuzix.

## Emulation

- ./smallz80 -i emu-ide.img

Boot
G, Enter

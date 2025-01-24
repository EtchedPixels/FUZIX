# Fuzix for a slightly modified Grant Searle board

## Modifications

The first modification is to use the W/RDYB pin to control A16 of the 628128
(pin 2) which is grounded. Bend up the pin, and wire it to W/RDYB (30) on
the SIO. Wire a 4K7 pull up to that link. This trick was taken from Bill
Shen's Simple80 design.

The second modification is to bend up DCDA on the SIO-2 and wire it to an
8Hz square wave source. The board as designed has no timer source as CP/M
2.2 doesn't actually need one. There are plenty of ways to make an 8Hz
square wave circuit, including these days just using one of the tiny PIC
devices to count it.

(Other frequencies will do but I happen to own an 8Hz source). Set the
clock frequency in config.h and fiddle as needed with devtty.c as the
kernel needs to see a clock divisible by 10 so some fudging is done).

## ROM

Finally you need my revised ROM that copies a bank to bank helper into both
RAM banks before paging the ROM out. See monitor.diff for the patch to the
supplied monitor.asm.

Assumes console is SIOA (the TTL port)

## Installation

make diskimage

disk.img is a raw CF image of a partitioned CF disk

Boot the machine and in the monitor run the CPM command.


## Emulation

Write the bootloader once to block 0 (repair the parition tables afterwards)
Write kernel images to block 24
Make it into a disk image

searle -r scm.rom -i fuzix.ide -b -t 


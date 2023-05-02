# Notes for RC2014 Tiny

## Bugs

If you have an early style ACIA then there is some junk on the screen at
boot from probing the CTC (which overlaps the addresses). This is not
harmful just ugly.

## To Add

* Flow control on the serial port
* Look at fast SIO and ACIA unbanked serial buffers

## Memory Map

Kernel mode

0000-7FFF	Kernel (and bootstrap)
8000-BFFF	Discardables / Userspace
C000-FFFF	Kernel

User mode (one process in memory at a time, swap mandatory)

0000-BFFF	User space
C000-FFFF	Kernel

## Notes

The memory map is very tight - be careful not to overrun the low 32K ROM
with code. Any changes may need more code hoisting up into the common
space.

## Emulation

ACIA / DS1302
- ./rc2014 -p -R -r fuzix.rom -i emu-ide.img

SIO / DS1302
- ./rc2014 -p -R -s -r fuzix/rom -i emu-ide.img

SIO / CTC
- ./rc2014 -p -c -s -r fuzix/rom -i emu-ide.img

# Ohio Scientific Challenger III

A CPU switchable machine based on their 6502 systems.

## Memory map

Kernel
0000-BFFF	Kernel		(banked)
C000-CFFF	I/O		(all banks)
D000-DFFF	Kernel		(common)
E000-EFFF	Hard Disk	(dual port RAM)
F000-FFFF	I/O and ROM	(common)

User
0000-BFFF	Kernel		(banked)
C000-CFFF	I/O		(all banks)
D000-DFFF	Kernel		(common)
E000-EFFF	Hard Disk	(dual port RAM)
F000-FFFF	I/O and ROM	(common)


## Hardware:

Model 510 CPU card
48K banked memory (at least 3 banks needed, more recommended)
4K  unbanked at D000-DFFF
Floppy card	(only used for timer and boot right now)
CD36/CD74 hard disk	(hardcoded at the moment)

## TODO

Can we optimize disk writeback by looking for other blocks that are
in the buffer cache for the chunk we will write and doing them too
providing not in swap mode ?

Init and test for ACIA devices on other ports, set them to a safe dummy
if absent (2 bytes RAM as a "not busy" ACIA)

Why doesn't interrupt work on Z80 ? (emulator issue ?)

Floppy disk driver

Other disk sizes and make them a config option

Driver for older disks 7MB etc (plus build a smaller image for them)

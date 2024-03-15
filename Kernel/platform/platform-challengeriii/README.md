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
48K banked memory (at least 3 banks needed)
4K  unbanked at D000-DFFF
Floppy card	(only used for timer and boot right now)
CD36 hard disk	(hardcoded at the moment)


At this point we can boot to the boot prompt and read the partition table
but trying to mount the root fs fails.

The PIA code for memory mapping needs some changes after that

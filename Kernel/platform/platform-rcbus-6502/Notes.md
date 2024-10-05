# RCBus 6502

## TODO

Although the memory card can operate as 4 independent 16K pages for simplicity
at this point we treat it as if it just provided 16K and 48K

## Interrupts

Interrupts deal wth the shared ZP by swapping the ZP back and forth with the
interrupt copies (about 20 bytes involved)

## Memory Map

Memory map:
	0000-00FF		Zero page
	0100-01FF		Stack
	0200-1FFF		Common, constants, cc65 support code
	2000-3FFF		Initial and discardable boot time
	4000-FDFF		Kernel
	FE00-FEFF		I/O window
	FF00-FFFF		Vectors (mostly free)

User space:
	0000-00FF		Zero page
	0100-01FF		Stack
	0200-1FFF		Common, constants, cc65 support code
	2000-FDFF		User space
	FE00-FEFF		I/O window
	FF00-FFFF		Vectors (mostly free)

The memory banks are allocated as follows

	0			Bootstrap ROM
	1-31			ROM (not used - may one day hold ehbasic
				and other OS stuff)
	32			RAM mapped initially at 0-16K
	33			RAM mapped initially at 16-32K
	34			RAM mapped by the bootloader at 32-48K
	35			RAM mapped by the bootloader at 48-64K
	36-63			Handed out in fours for process maps

## Quirks And Oddities

The 6502 port has some interesting gotchas to be aware of and things that
need addressing

We share the zero page and stack between user and kernel. The interrupt
path either has to have its own bank or copy/restore some state to make
interrupt handling in C work. For this port we don't waste 16K on an
interrupt bank but we could in theory. Whilst the 6502 hardware stack is
a precious resource the compiler makes little use of it so this seems to
be fine.

The common area is duplicated into each process common space. This means
any writes to this area are effectively process specific. For the udata it
is exactly what we want, for other things be careful.

There is a direct relationship between kernel and user on some ZP usage. In
particular the user C stack zero page locations are used by the kernel to
access arguments. The relocator ensures the kernel and user share the C
stack ZP locations so the applications are portable.

Signals are tricky. The C stack is maintained via ZP and is 16bits. As there
is no way to know if the stack was mid update an interrupt cannot tell if
the C stack is valid. The signal handlers deal with this by moving 256 bytes
down the C stack which should always land in a safe spot even if mid update.

Great care is taken to ensure nothing assumes ZP is at 0. On some 65C816
systems it won't be.

System calls are done via a JSR. BRK may look good but it's not reliable on
NMOS 6502 parts.

The cc65 support code is of reasonable size and actually the same for user
and kernel code. Some kind of shared library scheme for it would make
binaries a lot smaller.

There is minimal support for 65C816. The entire OS runs in 8bit 6502 mode
except for using MVN in fork, and some bits in the C library where setjmp
and longjmp need a minimal awareness of 65C816.

## TODO

- Swap support
- Look at other RCBUS device support


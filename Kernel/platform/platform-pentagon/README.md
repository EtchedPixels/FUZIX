# Fuzix for the Pentagon 256/512K systems

For the 128K machine use the 128K spectrum targets.

## Supported Hardware

Pentagon 256/512K system or compatible with the additional memory bank selects
as bit 7/6 of port 0x7FFD. This will not work for some of the other clones
as they use other ports (ATM for example uses FDFD low bits, ZX Profi uses
DFFD and the Phoenix 1FFD).

NemoIDE disk controller

Turbo bits are not set - they just vary too much by machine variant, so you
may want to add them to your build.

## Installation

Make diskimage will produce an IDE disk image and HDF file for emulators along
with a .trd file for the boot disk. Boot the floppy from Gluk on a hard
reset.

## Notes

These machines all have the same basic problem, there is a 16K window at
C000-FFFF which is pageable but no bigger pageable range.

We run with the following mapping

0000-3FFF	System ROM (fixed)
4000-7FFF	Kernel data/common
8000-83FF	Needed to create the exec of init
8400-BFFF	_DISCARD area - blown away when we exec init and becomes user
C000-FFF3
	0:	Kernel CODE1
	1:	Kernel CODE2
	2:	Mapped at 0x8000-0xBFFF (holds current process copy)
	3:	kernel CODE4
	4:	User process
	5:	Mapped at 0x4000-0x7FFF (kernel data/common)
	6:	User process
	7:	Kernel CODE3, Video
	8-15:	User process
	(32-63: User process on 512K machine)

FFF4-FFFF are needed for IRQ vectoring so we keep the udata copy a page
lower than normal.

User processes live in 4/6 and 8+. We have copy stuff back and forth
to page 2 due to the memory manager limits.

It would probably be fairly easy to add ATM support using a different variant
of switch_bank and size_ram, but the extra and better suited video modes
would probably make a separate related target a better option.

## Beta disk

The floppy controller is only visible when the floppy ROM is active in the
low 16K and is disabled when an instruction fetch occurs above 0x3FFF. This
coupled with the lack of suitable ROM helpers in most ROMs means the unit
is not supported. It is technically possible to support it using stupid
hacks on machines that can map RAM in the low 16K but not on the classic
Pentagon without a custom ROM. It's therefore not supported.

## To Do
-	tricks-big and core - best way to use the 512 byte uarea zone
	and nibble off FFF0-FFFF for the interrupt hooks
-	Why does ls /bin lose the lowest pixel row of the 'yes' command ?
	(Seems we lose the bottom line of the left most char - off by one bug
	 in zxvid ?)
-	6 or 5bit wide fonts (42, 51 column)
-	Optimize zxvid - especially scrolling
-	Move vtborder to common
-	See if we can in fact keep 6000-7FFF clear. If so we can look at
	ldir switching another 8K up and down to give 40K program sizes
	which would be a big improvement. Would need us to allocate 2.5
	banks per process however so need some allocator changes

Floppy disk interfaces are problematic. The standard Betadisk interface locks
the I/O ports to its ROM being active, which sucks.

## Emulator

You will need an emulator that can handle NemoIDE and the Pentagon style
I/O decode properly. FUSE will not cut it and the maintainers ignored the
patch that added support.


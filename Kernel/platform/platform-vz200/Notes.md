# Lazer VZ200 with 128K + SD card

## Memory map:
0000-3FFF	ROM (fixed)
4000-67FF	Populated by one of two banks from SD card SRAM
6800-6FFF	I/O latches and keyboard
7000-77FF	Video RAM (may be paged on modded machines). Only low
		512 used in normal text mode. Contended access causes display
		noise. If the graphics mods are fitted then it's an 8K RAM banked
		2K x 4 banks
7800-8FFF	Onboard memory (not paged)
9000-FFFF	Two SRAM banks off 128K card

### Kernel
4000-67FF	Kernel low (two banks)
7000-71FF	Video
7200-77FF	Video space (starts as part of discard)
7800-87FF	Common (only discard overlaps non text video)
9000-FFFF	Kernel main (ideally 9000-FFFF so can get bigger user space)

### User
8800-FFFF	User space
7000-71FF	Video	(7000-77FF after boot)

## TODO
-	Looks like there is a late reference somewhere into discard/font space we
	need to find and fix (corrupted "=" in font)
-	Optimise the SD read/write by unrolling by 7 (for 511 iterations) so we
	can also overlap the djnz with an out/in cycle
-	Tune as best we can for avoiding video flicker
	- Move to video in the main execution thread batching ops to the
	  irq handler - queue up 3 byte sets of addr,byte with codoes
	  for scrolls and clears to run through
-	Graphic modes in devgfx

The floppy interface is not supported. This shouldn't matter as the floppies
are weird, incredibly slow and the floppy DOS will clash with the SD.

## Interrupts
The system ROM jumps to 0x787D. We juggle our discard and common space to cope

## Memory

Memory is incredibly tight, especially in the common space. Do not add stuf without
careful checking.

DISCARD starts at 7200 so we can throw it for video
COMMON follows it and must start at above 7880
COMMON must end before 8800 (start of user)
The two banked code segments must fit between 4000 and 6AFF
The main code segment must fit between 9000 and FFFF

We could switch to a LEVEL_0 build to get a bit more space back but right now it
just about fits

In order to get interrupts sane we disable interrupts on boot and keep them off
until user space has started, at which point we blow away discard which leaves
7800-787F free, along with the rest of video and probably some more above 7800 we
have no particular use for. We then write our handler into 787D in the usual fashion
except that we undo the ROM behaviour and jump to our code so we can pre-empt properly
and so on.

Post boot all the video space is free. So for a normal machine the usual video modes
are available. For banked video we switch to 256x192 and hide the font in the final
2K bank the 6847 never sees.

Flicker may be a big problem though.

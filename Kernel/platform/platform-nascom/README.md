# Fuzix for a NASCOM with paged mode memory

## Supported Hardware

NASCOM
Paged RAM cards (3 or 4 pages required) - eg GM806, GM862
Z80PIO wired to a CF adapter
MM58174 based RTC (only used for time sync and NMI based tick)

## Memory Map

0000-BFFF		RAM B (can be pageable)
C000-EFFF		Can be off a RAM card (even pageable) or from mainboard ROM/RAM sockets
F000-F7FF		BOOT ROM
F800-FBFF		Video RAM
FC00-FFFF		Kernel stacks and common data

## Hardware Support

- Keyboard
- Video
- Serial
- PIO CF adapter
- GM816 RTC (or similar) as timer only, can use NMI (jumper LKS2 7-8)
- GM833 RAMDisc
- MAP80 (1 to 4 cards)
- Floppy (Needs more testing on the variants)

Note that for the MAP80 you need to set CONFIG_MAP80 in kernel-z80u.def
and config.h and do a clean and rebuild of the kernel. The MAP80 build at
this point uses the same memory map as the page mode, whereas it (and some
of the GM page mode cards) could do 0xE000.

## Options

If you have an RTC at 0x20 wired to NMI then specify "rtcnmi" on the command
line along with the boot device number to enable it.

## TODO 
- Probe top of bankable space and adjust top of user memory to match
- Report correct ramsize based on bankable space size
DONE? - Sort out remaining keyboard bits - *, ctrl etc
- Floppy (nascom or gm ? - WIP)
DONE - Finish rewiring nmi key handler
DONE - Implement RTC NMI timer interrupt
- Normal interrupt handling for timer on PIO bit
- SD instead of CF (will be very slow though)
- Full RTC
- MAP80

## Longer Term : Add on cards
- GM816 CTC for proper timer interrupt (and IM2 support)
- Second CF on GM816 PIO
- AVC video
- MAP80 video
- Gemini SASI/SCSI
- Henelec disk on PIO
- RTC on PIO

## Installation

make diskimage produces a boot floppy image (single sided) and a CF card
image as well as an emulation version of the latter. Boot with the standard
boot rom from the floppy. On boot hda1 is the root file system and hda2 is
swap.

## Limitations

There are several mm58174 based cards available that can provide time. Most
are not capable of providing an NMI so will shut up if not poked within 16ms
of an interrupt event. This makes them unreliable so for now not supported.
Maybe checking for missed interrupts versus the time value would help but
the question is where to do so.

The same problem occurs for NMI based cards when using a flopppy but we
need to disable it during the transfer anyway so this is ok and handled.

At the moment we could use non NMI ones for clock synch only. The homebrew
ones seem to use 0x20 as well. The GM822 attaches to any PIO you have handy.
Conventionally this is the base system PIO (the one we use for IDE). For the
GM822 see 80bus news issue 14. It can only provide IRQ not NMI.

When running with a clock as timer only without NMI the system effectively
runs with co-operative multitasking.

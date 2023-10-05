# Fuzix for a NASCOM with paged mode memory

## Supported Hardware

NASCOM
Paged RAM cards (3 or 4 pages required) - eg GM806, GM862
Z80PIO wired to a CF adapter
MM58174 based RTC (only used for time sync right now)

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
- GM822 RTC (or similar) as timer only
- GM833 RAMDisc

## TODO 
- Sort out remaining keyboard bits - *, ctrl etc
- Floppy (nascom or gm ? - WIP)
- Finish rewiring nmi key handler
- Implement RTC NMI timer interrupt
- Normal interrupt handling for timer on PIO bit
- SD instead of CF (will be very slow though)
- Full RTC
- MAP80 card and maybe combos

## Longer Term : Add on cards
- GM816 CTC for proper timer interrupt
- AVC video
- Gemini SASI/SCSI

## Installation

make diskimage produces a boot floppy image (single sided) and a CF card
image as well as an emulation version of the latter. Boot with the standard
boot rom from the floppy. On boot hda1 is the root file system and hda2 is
swap.


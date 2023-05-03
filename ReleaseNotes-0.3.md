# Fuzix 0.3 Release Notes

Fuzix 0.3 is the third major release of the Fuzix OS. This release has mostly
been focussed upon kernel improvements and supporting more platforms,
particularly ZX Spectrum derived ones. The 68000 code base is now usable
but not entirely debugged and optimized. 8080 and 8085 support has been added.

## Documentation

There is now a documentation directory and rules to build it
into something. It's still very incomplete and badly formatted

## Warning

Fuzix 0.4 will break ABI compatibility big time. There are a pile of bad
and historic design decisions and behaviours that need to be dealt with.


## User Space Changes:

New Commands: tty, vile, sok, fweeplet (Zork engine), fsh (sh with editing)
Main Improved commands:
	cp,mv,ln now taken from Heirloom so full Unix style commands
	dd can use stdin/stdout
	fweep now generally available
	sh now correctly handles out of memory
	ucp now behaves correctly if you ls a file.
	sort is now based on a different smaller codebase.
	fsck check on a clean file system is much faster.


## Behavioural Changes:
- Some systems had fd first not hd first. This has been unified so on
  platforms where 0 was fd0, it's now 256 and hda is 0. If you use the fd
  or hd names at boot prompts then it's not visible

## Systems Added

- Amstrad PCW 8256 with CF adapter (minimal early port)
- Bill Shen
--	SBC64/MBC64
--	Simple80 (in test)
--	Tiny68K (for 68K development - very glitchy still)
- Cromemco with 16FDC and 8" disks
- Easy-Z80
- Grant Searle Z80 CP/M design with small modifications
- Linc80 with extra 16K RAM card
- Pentagon 1024 with NemoIDE
- RC2014 with 6502 Processor (emulation only board debug in process)
- RC2014 with 8085 Processor
- Sam Coupe (minimal port only at this point), Atomlite IDE
- Small Computer Central
--	SC108	Z80 128K RAM, CF
--	SC111	Z180, 512K RAM, CF
--	SC114	Z80 128K RAM, CF
- SBC2G (Grant Searle style system with banked memory)
- Scorpion with NemoIDE
- Scrumpel (Z180)
- SmallZ80
- Timex TC2068/TS2068 with DivIDE and Fuzix on a cartridge
- Tom's SBC (with small mods)
- Video Genie with EG64B banker (and in theory TRS80 + Lubomir soft banker)
- Z80-MBC2
- Z80 Membership card
- ZX Spectrum 128K with DivIDE or DivMMC
- ZX Spectrum +3 with ZXMMC or similar

## Existing Supported Systems

- Amstrad NC100 with 1MB SRAM card
- Amstrad NC200 with 1MB SRAM card and floppy disk drive
- Dragon 32/64 with Spinx or MOOH cartridge
- DX Designs P112 with 1MB RAM and G-IDE
- EACA Video Genie (with suitable banked memory)
- LNW Research LNW80 with suitable banked memory (TRS80 Model 1 compatible bits)
- Memotech MTX512 with SDX disk (no Rememorizer or Rememotech support)
- Microbee 256TC or Premium with 128K+ RAM and ideally hard disk
- Multicomp09 with at least 128K RAM
- RBC(*) Mark IV with optional PropIO V2
- RBC(*) SBC v2 with optional PPIDE and/or PropIO V2
- RC2014 SBC with 512K ROM/RAM, CF, SIO and RTC
- RC2014 SBC with banked ROM, 64K RAM, CF, SIO and RTC
- SOCZ80 (128MHz FPGA Z80 platform)
- Tandy COCO2 with Cloud 9 IDE, or COCOSDC card and Fuzix partly in cartridge
- Tandy COCO3 with suitable disk interface
- TRS80 Model I/III with a hard disk and a supported banked RAM expansion
- TRS80 Model 4/4D/4P 128K RAM
 -Zeta
- Zeta V2

(*) Formerly N8VEM now RBC (RetroBrewComputers)

## Incomplete Ports

- Apple IIc	-	initial investigations
- Atari ST	-	in progress
- EZRetro		-	minimal development port for ez80
- Gemini		-	early design
- Lucas Nascom	-	early design
- MSX1		-	works in very limited configuration only
- Pentagon	-	needs the 0.4 ABI changes to be possible
- ZX Uno	-	for now run the 128K ZX Spectrum version as this also
			knows about some Uno features
- Z80BIOS	-	experiment to see if things like S100 can be supported
			by having a Z80 BIOS akin to the CP/M BIOS etc.
- Z280RC/ZZ80RC	-	early sketches


## New Virtual Platforms (Emulation Platforms For Fun or Development)

- v8080		-	8080 development environment
- v85		-	8085 development environment

## Existing Virtual Platforms

- V65C816		-	65C816 emulation work
- V68		-	68000 development work
- Z80Pack		-	Very flexible Z80 banked system emulator

## Obsolete

- MSX2		-	needs a major rework and clean up

## Major Changes To Existing Platforms

- Dragon-MOOH	-	now it's own port with full MOOH paging support.
- MTX		-	support for CFX-I CFX-II, some rememorize support, base MTXPlus support, extra keys on PC adapter. Lots of bug fixes.

## The Library/Application support has been built for

- 6502			no FP
- 65C816			no FP, using 6502 modes
- 6809			FP needs debug
- 68000			FP needs debug
- 68HC11			test only
- 8086			test only
- MSP430X
- NS32k			test only
- PDP11			test only
- Rabbit2000		test only
- Z80
- Z180
- eZ80			needs a very recent SDCC

## Core Changes

- New memory banking model for 16K banks with a fixed common
- Timer handling optimized massively.
- Correct handling of platforms with an optional RTC when it is not present
  (previously clock would drift or not run)
- Correct problems if the console had carrier detect and could be hung up
- Fix races in carrier and hangup logic
- Clear error bits correctly on Z80 SIO
- Printer ioctls added
- 6502 platform support now complete including signal handling
- eZ80 support
- Z80 memory model support for machines with no usable common RAM
- Z80 switch to a software interrupt disable tracker rather than doing the expensive NMOS Z80 workaround
- Z80 memory model support for 32K/32K split
- 8080/85 platform support complete
- 68000 flat memory model
- /dev/sys ioctl framework for platform / cpu specific ioctls
- Unified lots of drivewire code
- Support for "swapon"
- Single process in memory systems not run parent first after fork. That usually means that we hit waitpid() and avoids an extra set of swaps.
- Base support for Z180 CPU dropped into a Z80 environment
- Clock set from root fs superblock timestamp on start up. This avoids clocks going backwards and usually means the date is right when you boot and get asked.
- Use uint_fast8_t for platforms where uint8_t is slow, or the compiler sucks at handling them.
- Move repeated swap helper into partition code.
- udata offsets are now properly offsets of a symbol so it isn't repeated in kernel.def and can't end up wrong.
- Support byte-swapped IDE
- Termios masking in the core code to make termios handling in drivers easier

Bug Fixes
- Z80 handling of NULL trap no longer crashes the system
- Fix crash case where we could swap out a zombie process
- Fixed some confusion around di and int_disabled state when making calls to banking code. A more general fixup will happen in 0.4
- Microbeee totally mishandled memory banking but happened to sort of work. The bank rules are now corrected.
- A failed fork corrupted memory.
- Correct various problems with '..' transition introduced in older changes
- Lots of platforms mishandled buffer reclaim of spare memory. Correct this and extract the code where possible.
- Fix a tty and pre-emption deadlock
- corrected bugs in valaddr() range checking
- Fixed termios handling bugs in init
- Fixed 5bit wide character default bug in boot up
- cpuinfo no longer faults on Z180

Library Fixes

- isatty is much cleaner and simpler
- ttyname is usually much faster
- device name finding routines sped up a lot for the usual case
- 6809 double is 4 bytes not 8.
- curses handling of ROWS/COLS fixed also of wgetch(). ERR is now -1 so control-A is not confused.
- New mini readline library

				---------


			Fuzix 0.2 Release Notes

Fuzix 0.2 is the second major release of the Fuzix OS. The primary focus has
been completing the core C library functionality. Applications have also been
updated and the kernel has a considerable number of bugs fixed and new
platforms.

Supported Hardware

Amstrad NC100 with 1MB SRAM card
Amstrad NC200 with 1MB SRAM card and floppy disk drive
Dragon 32/64 with Spinx or MOOH cartridge
DX Designs P112 with 1MB RAM and G-IDE
EACA Video Genie (with suitable banked memory)
LNW Research LNW80 with suitable banked memory (TRS80 Model 1 compatible bits)
Memotech MTX512 with SDX disk (no Rememorizer or Rememotech support)
Microbee 256TC or Premium with 128K+ RAM and ideally hard disk
MSX2 + MegaFlashROM with SD
Multicomp09 with at least 128K RAM
RBC(*) Mark IV with optional PropIO V2
RBC(*) SBC v2 with optional PPIDE and/or PropIO V2
RC2014 SBC with 512K ROM/RAM, CF, SIO and RTC
RC2014 SBC with banked ROM, 64K RAM, CF, SIO and RTC
SOCZ80 (128MHz FPGA Z80 platform)
Tandy COCO2 with Cloud 9 IDE, or COCOSDC card and Fuzix partly in cartridge
Tandy COCO3 with suitable disk interface
TRS80 Model I/III with a hard disk and a supported banked RAM expansion
TRS80 Model 4/4D/4P 128K RAM
Zeta
Zeta V2

(*) Formerly N8VEM now RBC (RetroBrewComputers)

Virtual Platforms (Emulation Platforms For Fun or Development)

V65C816		-	65C816 emulation work
V68		-	68000 development work
Z80Pack		-	Very flexible Z80 banked system emulator

Obsolete/Dropped Platforms

MSP430FR5969 - bitrotted
MSX1 with MegaMem - bitrotted
TGL6502 - project died
ZX128 (Spectrum 128K etc) - too difficult to make work well

Incomplete Ports

-Amstrad PCW8256/8512/9256/9512/10
-Apple IIc
-Cromemco
-Dragon 32/64 MOOH (full MOOH support, use the Spinx/Mooh port for now)
-Gemini
-Lucas Nascom
-Sam Coupe
-SC108 (port complete for proposed hardware)
-V65 (Virtual 6502 development platform)
-Video Genie with EG64
-Z280RC


The Library/Application support has been built for
6502			no FP
65C816			no FP, using 6502 modes
6809			FP needs debug
68000			FP needs debug
68HC11			test only
8086			test only
MSP430X
PDP11			test only
Rabbit2000		test only
Z80
Z180



Changes From 0.2 to 0.2.1

o	Fixed a bug where zombies got swapped in error. Fixes the TRS80
	hang at the date prompt
o	Fixed a warning about banking fixups on the Model1
o	Better debugging in swap.c
o	Better debugging in binmunge
o	Remembered to add the release notes!

Changes from 0.2.1 to 0.2.2

o	Spell MOOH correctly

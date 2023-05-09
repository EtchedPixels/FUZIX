# Fuzix 0.4 Release Notes

** Work in progress as we move towards a release candidate **

## Overview Of Changes

The core of the Fuzix kernel remains much the same for this release. A
number of bugs have been fixed and some interfaces improved. The networking
layer has been completely reworked to be more modular so that in future it
can run in a different address space to the kenel on 8bit machines.

Executable formats have changed. The 8080, 8085 and Z80 binary formats are
now properly unified so that 8085 and Z80 can run 8080 binaries directly.
The 68HC11 and 6803 formats are somewhat different but the syscall ABI is
arranged so that the 68HC11 can run 6803 binaries.

The 32bit binaries that were using a bodged Linux binflt format are now
using a.out with some small extensions to handle the relocation maps. This
should hopefully now become a stable executable format for the future.

Building has, where possible, been made easier. The tool chains remain a bit
of pain because of the fact many are somewhat obscure, and those that are
not tend to get broken on a regular basis forcing specific releases to be
used. The actual system build however now has a "make diskimage" target that
puts together all the pieces for a bootable system in one go rather than
requiring the builder understands the finer details of the system in
question and how to merge all the pieces together.

The make environment is much better than it was. It is still terrible
however so a "make clean" is needed when switching processors and a "make
kclean" is strongly recommended when tweaking any kernel config options or
switching target. A lot of the make rules have been merged which should make
the problem of sorting out all the rules and dependencies more tractable for
0.5.

## Naming Changes

The N8VEM project rebranded as 'Retrobrew' at the request of its founder who
had ceased to be so involved. The 0.3 release fixed most N8VEM naming, 0.4
completes this.

There is now a distinction between RC2014 (the product line) and RCbus (the
bus standard). In particular the bus has been extended beyond the original
concept and now has its own standard document and keepers. Fuzix should now
only use the 'RC2014' nomenclature for official RC2014 products but there
may be a few that have been missed. Systems that were previously rc2014-xyz
are now rcbus-xyz.

## Supported Processors

### 6303 / 6803

Hitachi 6303 and Motorola 6803 processors. These are supported by the CC68
compiler chain and tools derived from cc65 and specifically designed for
Fuzxi. Currently the only target board is the rcbus 6803/6303 processor
card. Floating point is not supported, but adding it would only require
someone writes the basic underlying soft fp routines (add, subtract etc).

Other targets should not be hard to add.

### 6502 / 65C02 / 65C816

These are supported by cc65 (v2.18 or later). Due to compiler limits floating
point is not supported. 65C816 is treated as a 65C02 with extras because of
the lack of an open 65C816 compiler.

Currently this port targets the RCbus 65C02/65C816 cards and the PZ1. Most
of the "classic" 6502 systems have neither the memory or the I/O, and in
most cases even when they are upgraded with some of the late era processor
upgrades still lack decent I/O.

### 6809

This port uses gcc 6809 and lwtools. It supports a range of classic and
modern systems including Dragon, Tandy COCO, Thomson and RCBus machines.

### 68HC11

The final generation of the 6800 processor line this port uses gcc and has a
very different ABI to the 6800/6803. It can run 6800 and 6803 binaries
however, but not 6303. At the moment this port targets the Mini11 SBC and
the RCbus 68HC11 card.

### 68000

Motorola 68000 series processors with flat memory space. The port is now a
lot more stable and has a sensible binary format. Processors up to 68EC020
are catered for. An additional memory model has been added for systems with
low memory and it is now just about possible to run Fuzix on a 128K system
without relying on fast disks. GCC and binutils need to be built in a
particular way for pre 68020 processors otherwise they will insert
unsupported instructions. See the README for 68000.

### 8080

The 8080 is supported using the ACK C compiler at this point. The only
targets we build for are test emulated environments in order to check that
the output is good. The previous kernel used the 8080 build for 8085
systems but this has changed. The goal is to remove ACK entirely for 0.5
once the new C compiler can generate good valid 8080.

### 8085

This port now uses the full 8085 instruction set (including the stuff Intel
decided to not to document). It uses a new compiler built specifically for
Fuzix. This port is thus currently a little bit flaky and there are tool and
system bugs to nail down. Performance is several times faster than the pure
8080 build as the extra instructions make a huge difference when executing
high level languages. Floating point is not yet supported as it needs the
base low level FP routines for 8085 writing

The supported target is the rcbus 8085 card with onboard bank MMU.

### ARM

ARM M0 is supported using gcc and targetting the Raspberry Pi Pico. ARM M4
targets for the DK-TM4C129X.

### ESP8266

This target is specific to the ESP8266 variant of the Tensilica L106.

### NS32K

A complete port for MMUless NS32K processors. This port is new as of 0.4. It
targets the in-progress RCbus NS32FX16 processor card design.

## Z80 / Z180 / 64180 / Z84C1X

These processors are all supported by the main tree using a fork of SDCC
3.8. The newer SDCC changes a lot of calling conventions so no move to it
has been made at this point. That may change in the future.

Supporting code libraries make most flat Z180 systems trivial and provide
all the mapping and peripheral support.

## Other Processors

These are processors which are in the tree but not yet fully functional. In
some cases this is merely used to help catch portability problems in the
libraries.

### 6800

Work in progress to extend CC68 and Fuzix to the original 6800/6808
processor line. Not yet usable

### 8086/8088/80C188/80C186

This is currently just the base sketches for a future PC and Rcbus-80C188
port.

### ESP32

An initial experimental port only

### EZ80

Initial support for booting on an ez80 based platform. In this case the Jee
Retro development platform. This should form a good basis for enabling any
other ez80 platform, but none have been enabled yet.

### MSP430X

Retired

### PDP11

Tracking the state of the GCC PDP11 compiler and binutils. Not yet at a
point the toolchain works well enough.

### Rabbit 2000/3000

At this point used as a build check on the user space. Hopefully some Rabbit
board enabling will happen during the 0.5 work.

### RiscV 32

Tool chain testing and portability work. At the moment this is another
compiler chain that we break. In theory Fuzix should be able to run on some
of the upcoming and recent RiscV micro-controllers with 128K or so of SRAM.

### TMS9995

At this point being used to slowly debug the C compiler. Not a useable port
as the compiler is still a fair way from working correctly.

### WRX6

Warrex CPU6. Being used for compiler debug but also on hold until the
documentation of the system is in a better state to make progress.

### Z280

Work in progress. The Z280 is very Z80 like but the different privilege
structure and interrupt behaviour meand that for the Fuzix at least this
will need its own variant of the low level core code.

## Supported Systems

For more details on each system consult the relevant README.md in that
Kernel/platform-xxxx directory. There are othee platforms but if not listed
here they are likely works in progress or special cases.

### 2063

John Winan's 2063-Retro system as featured in his "John's Basement" youtube
series. A fairly classic Z80 Retro system with somewhat slow bitbang SD card
interface.

https://www.youtube.com/c/JohnsBasement

### 68K-nano

The 68K nano design from Matt Sarnoff. A minimalist 68000 system with 16bit
IDE interface. This makes a very nice and easy to build little Fuzix box.

https://github.com/74hc595/68k-nano

### Ampro Littleboard

The Ampro Littleboard was a classic Z80 CP/M board designed to be the same
size as a floppy disk. This port requires the Plus version of the board with
SCSI controller.

### Amstrad NC100

An pre-laptop portable word processing machine with PCMCIA slot.

### Amstrad NC200

The sequel to the NC100 with a floppy drive and much nicer display.

### CPM22

A port that uses a customisation block plus the CP/M 2.2 BIOS. Intended to
make it practical to port Fuzix to systems like S.100 machines where the
BIOS is basically the architecture and each machine tends to be a bit
unique. If you need to port Fuzix to a platform and do not want to deal with
the Fuzix and C side of things this may be a good basis. At the moment it is
very basic, but easily extensible to cover other gaps in the CP/M BIOS.

### COCO2

Support for the Tandy COCO2 with 64K RAM and CF or SD card adapter. The
kernel is partly flashed onto a cartridge bank to make it all fit nicely.
Should also work with a Dragon 64.

### COCO3

The Tandy COCO3

### Cromemco

Z80 based system with 8" floppy disk interface. The hard disk is not
currently supported due to lack of documentation/example code.

### DK-TM4C129X

An ARM development board

### Dragon 32

There are two ports specifically aimed at the Dragon machines. Each supports
one of Tormod Volden's SD and memory extensions - the NX and the later more
featured MOOH. This port should, with the right build options, also work on
COCO machines with the same interface.

http://tormod.me/mooh.html

### Dyno

A fairly generic flat memory Z180 retro system.

https://homebrew.computer/dyno-computer/

### Easy-Z80

Sergey Kiselev's EasyZ80. Very similar to the RCbus and RC2014 systems with
512/512K RAM but integrated onto a single board with battery back up and
also IM2 interrupt mode support.

https://github.com/skiselev/easy_z80

### ESP8266

Fuzix for the ExpressIf microcontroller. The onboard wireless is not
supported because a) it's not documented and b) we stole all the memory it
uses and repurposed it. It does however support using a WizNet 5500 for
internet connectivity.

### Genie EG64/3

Support for the Video Genie (aka Dick Smith System 80, PMC-80 etc)  with
the Genie EG64/3 CP/M adapter or the equivalent TRS80 Lubomir soft banker.

### KC87

Robotron KC87, KC85/1 and Z9001. Not the KC85/4. East German systems
produced by Robotron-Meßelektronik.

### MB020

A simple flat memory 68020 board by Bill Shen

https://www.retrobrewcomputers.org/doku.php?id=builderpages:plasmo:mb020

### Micro80

A retro Z8C415 based machine using the single chip integrated Z80 and I/O.

### Microbee

The classic Australian system yet almost unknown elsewhere. There are a long
series of these machines with growing power. At least 128K and some kind of
decent disk interface is required. Colour is recommended as the Fuzix kernel
does not try and deal with video update flicker on the older video.

### Mini11

Low chip count 68HC11 SBC with 512K of RAM driven directly off a 68HC11, and
the upper address pins driven by the 68HC11 GPIO lines.

https://github.com/EtchedPixels/Mini11

### MSX

The are two MSX ports. The MSX1 port is a cartridge based port that can run
on any 64K mmeory machine with Sunrise style IDE. There is no support for
things like the MegaRAM at this point.

The MSX2 port requires V9938 or higher video, MSX2 style memory and at this
point a MegaFlashROM with SD card. There is no support for MSX1 systems with
the orignal video and MSX2 memory banking although that would be good to add.

### MTX

Memotech MTX512 with banked memory and suitable disk adapter. An obscure but
rather beautiful British machine.

### Multicomp09

FPGA based retro system.

### N8

The N8 is a fusion of Z180 retrocomputer and sort of not quite MSXish
things.

https://www.retrobrewcomputers.org/doku.php?id=boards:sbc:n8:n8

### P112

DX Designs P112. An early 'retro' system using a Zilog ESCC.

### PCW8256/512/9256/9512/10

The classic Amstrad wordprocessor / CP/M machine. Still somewhat of a work
in progress. Note that the PCW16 is a completely different architecture and
not supported.

### Pentagon

Easten block ZX Spectrum clone. This port is targetted at systems with
256/512K of RAM. For the 128K machines just use the 128K Spectrum targets

### Pentagon 1024

The 1MB Pentagon with additional mapping capabilities

### Pico68K

A breadboard 68000 system using an ACIA and VIA plus bitbang SD card.

https://hackaday.io/project/179200-68000-minimal-homebrew-computer

### RBC Mark 4

The Retrobrew (formerly N8VEM) Mark 4. A Z180 based design for the ECB bus.

https://www.retrobrewcomputers.org/doku.php?id=boards:sbc:z180_mark_iv:z180_mark_iv

Note that 0.4 now requires RomWBW firmware.

### RBC Mini M68K

John Coffman's mini 68K system.

https://www.retrobrewcomputers.org/doku.php?id=boards:ecb:mini-68k:start

### RC2014

There are two Fuzix ports aimed at "official" RC2014 targets.

The RC2014 port requires a 512K RAM/ROM card and supports the bigger RC2014
configurations along with a lot of other compatible RCBus hardware.

The RC2014-tiny port is a "because we can" port that puts the core of the
Fuzix kernel in the pageable ROM and pages it in and out in order to run
Fuzix on the 64K + pageable ROM setup. This works but isn't recommended.
Note that you can (and should!) jumper the ROM card for a 28C256 not a
27C256 part if you are doing this, that way you can quickly erase it and
update it.

### RCBus (Z80 and compatible)

The following RCbus compatible systems have their own ports referenced
elsewhere in this document.

- 2063 (with adapter)
- Easy Z80
- RIZ180
- SC108
- SC111
- Simple80
- T68KRC
- ZRC

The SBC64/MBC64/ZRCC have their own rcbus-sbc64 target. In general the other
Z80 boards are compatible and operate with the RC2014 tree. This knows how
to handle Z180 processor cards with the banked memory card, and various
variant systems and I/O options.

The rcbus-z180 target handles the many near identical flat Z180 designs that
use the RCBus with a 512/512K flat 20bit memory space as well as the
modification sometimes used to allow for 1MB RAM.

### RCBus (Other)

There are specific ports for RCbus systems with the following processors:
6303 (and 6803), 6502, 6809, 8085, 68008, NS32K. The card requirements vary
by port. Please consult the port documentation for detail.

### Rhyophyre

A Z180 SBC with uPD7220 GDC video. The video is not supported in Fuzix at
this time.

https://github.com/lynchaj/rhyophyre

### RIZ180

A minimal system Z180 design by Bill Shen

https://www.retrobrewcomputers.org/doku.php?id=builderpages:plasmo:riz180:riz180r1

### Raspberry Pi Pico

Support for the Pi Pico embedded ARM board

### Sam Coupe

A "super ZX Spectrum" machine that proved to be too little, too late to
survive the shift away from 8bit home computers.

### SBC 2G

A banked memory design that draws heavily on Grant Searle's machine.

### SBC v2

Retrobrew SBC v2. Z80 base ECB bus card with simple banked memory. This
target is documented heavily and is designed to be a reference for anyone
trying to understand how to port Fuzix.

### SC108

Small Computer Central design with CPU, RAM and OM on one card.

https://smallcomputercentral.com/sc108-z80-processor-rc2014/

### SC111

Small Computer Central Z180 design. This tree has been kept apart from the
unification of various related Z180 RCbus systems because it also supports
the ability to boot from SCM firmware. With RomWBW firmware the SC111 can
run either.

https://smallcomputercentral.com/sc111-z180-cpu-module-rc2014/

### Scorpion

Another Eastern block ZX Spectrum clone that differs somewhat from the
Pentagon designs.

### Searle

Fuzix for a slightly modified version of the Grant Searle classic design.
The modifications consist of a single wire and resistor mod to use the full
128K of RAM and a tweak to allow an external timer to provide a timer tick.

### Sinclair ZX Spectrum

There are multiple ports for the variants of this system. For the clones and
not quite compatible systems please see
- Pentagon
- Pentagon 1024
- Scorpion
- TC2068

The following configurations have ports

#### 128K or later with DivIDE/DivMMC

This port uses the DivIDE/DivMMC memory in low space in order to run Fuzix
on the Spectrum systems that lack the ability to map RAM low directly. This
port also knows about the ZX-Uno extensions and will use them.

### 128K or later with SpectraNet and a disk controller

Similar to the DivIDE/DivMMC this port uses the SpectraNet memory in the
same way, and needs a disk controller such as a ZX-MMC to go with it.

### +2A or +3 with ZX-MMC or similar disk controller

Using the additional memory features on the +2A and +3 machines in order to
do rather better memory banking.

### Simple80

Bill Shen's glueless Z80 minimal design. Requires either a rev1 board or a
small board mod to correct the memory banking error in the original.

https://www.retrobrewcomputers.org/doku.php?id=builderpages:plasmo:simple80

### SmallZ80

TG Consulting Z80 system

### SocZ80

High speed FPGA based Z80 (T80) platform. Runs at 128MHz and supports 8MB of
RAM.

https://sowerbutts.com/socz80/

### TC2068

Timex reworking of the Sinclair ZX Spectrum. Incompatible, buggy and a bit
of a flop. It does however have support for RAM/ROM cartridges and has a
nice 512 pixel wide video mode.

### Tiny68K / T68KRC

A 68000 based system with 16MB of RAM and a CF interface. The T68KRC has
less RAM but an RCbus connector. Both are supported.

### TO8

Thomson TO8. An initial port to the TO8 platform.

### Tom's SBC

Z80 retro computer design with banked EPROM and 64K (or 128K with mod) RAM.
Fuzix can run from banked EPROM for a ROM based system or from RAM with the
128K modifier.

### TRS80

Tandy model 1 and model 3 systems (and clones) with an Alpha Supermem or Selector are
supported along with the Model 4. The 128K model 4 is supported without a
memory extender although the Dave Huffman style mod is supported. The XLR8R
is not directly supported at this time.

### VZ200

VTech Laser 200 with the SDDrive adapter.

### YAZ180

Yet another Z180 system.

https://github.com/feilipu/yaz180

### Z1013

An Easten block design and probably candidate for 'worst home computer
keyboard ever shipped'. Fuzix supports a suitable configuration which in
practice probably means the modern re-creation.

### Z50Bus

For all practical purposes the Z50Bus systems except the Linc80 are software
equivalent to the RCbus ones and use the same kernel. The Linc80 has its own
port but that requires building a DIY memory expansion.

### Z80 MBC-2

A small Z80 based system with an Arduino I/O subsystem.

### Z80 Membership Card

Sunrise EV stackable retrocomputer. Fuzix requires the CPU/RAM/SD card
combination.

http://www.sunrise-ev.com/z80.htm

### Z80Pack

Z80 emulated CP/M platform. Useful for debugging and development purposes.

https://www.autometer.de/unix4fun/z80pack/

### Z80Retro

Peter Wilson's Z80 Retro design

https://github.com/peterw8102/Z80-Retro

### Zeta V2

Sregey's floppy disks sized Z80 system with banked memory, PPIDE and floppy
controller.

http://www.malinov.com/Home/sergeys-projects/zeta-sbc-v2

### ZRC

Bill Shen's ZRC. A minimal Z80 system using a CPLD. The machine has 2MB of
DRAM. Fuzix has no idea at this point how to use it all because it's not
clear what you need 2MB for on a small Z80 system with no graphics.

https://www.retrobrewcomputers.org/doku.php?id=builderpages:plasmo:zrc


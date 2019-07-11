# Supported Platforms

## Classic Systems

### Amstrad NC100 and NC200

This requires an Amstrad NC system with PCMCIA memory card. On the NC100 the
OS is booted from the memory card image and the filesystem is kept upon the
memory card as well. The NC200 also supports the floppy disk drive.

Supported Features
- Keyboard
- Screen
- Serial port
- PCMCIA memory card
- Floppy disk (NC200 only)
- RTC (in test)
- Sound (only experimentally)

Unsupported
- Ranger serial floppy disk drive
- Related clones (some may work but the 1.44MB drive used by certain units
  may need additional work)

Tested On 0.2
- NC100: No
- NC200: Yes, real system

### Dragon 32 / 64 / Tandy COCO2

There are three different ports to differing configurations of this system

The first port is the coco2cart port. This requires a 64K Dragon or 64K Tandy COCO
with a Cloud 9 compatible IDE or COCOSD cartridge and FUZIX installed on the cartridge and disk.
FUZIX boots from cartridge, loads the rest of itself into memory and runs in
single process at a time mode. Due to lack of resources this port is quite
minimally featured but has some space for further improvement.

The second port is the dragon32-nx port. This supports the Dragon 32/64 or
Tandy COCO with at least 32K of memory and with Tormod's Spinx cartridge
giving 512K of banked RAM and an SD card slot. This port offers a full
featured FUZIX but with userspace limited to 32K per application. On 6809
this is not a big problem in general.

The third port is the dragon32-mooh port. This supports the Dragon 32/64
with Tormod's MOOH cartridge. This cartridge offers flexible memory
management much like that of the Tandy COCO3 and the port makes full use of
this.

Supported Features
- Keyboard
- Video (only text mode on coco2cart)
- Serial Port (Dragon only)
- SD or IDE interface
- MPI (not coco2cart)
- Drivewire
- 9128 CRT (not coco2cart)
- Floppy disk (not coco2cart)
- Printer

Tested On 0.2
- COCO2 with SDC: Yes - real system
- Dragon32 with NX: Yes - real system
- COCO2 with IDE: Yes - emulation

## Memotech MTX-512

The Memotech is a mostly UK system designed to run both as a games machine
or with expansions as a CP/M system including memory banking. To run Fuzix
you require at least 3 48K banks of memory plus common, and also a suitable
disk controller.

Supported Features
- Keyboard
- Video (40 column, SDX 80 column, propellor 80 column(partial))
- Serial Port
- CFX or CFX-II IDE interface
- Rememorizer SD (untested)
- MTXplus RTC (untested)
- SDX floppy disk
- Printer

Unsupported
- MTXplus video (at this point.. WIP)
- Rememorizer (although it probably is not much work)

## Microbee

Microbee produced a range of Z80 based systems primarily in and for the
Australian market. Fuzix requires a system with at least 128K of RAM, and
preferably a hard disk interface or modern CF adapter. Both the classic
keyboard interface and the 256TC are supported.

Supported Features
- Keyboard
- Display (colour, premium or 256TC)
- RTC (required and PIO B7 must be set to the RTC)
- WD1010 disk interface (floppy and hard disk)
- WD2793 disk interface
- Switching disk interface via 0x58
- IDE CF

Unsupported
- Z80 PIO serial
- Z8530 SCC option
- Mono display (will flicker on update)

Program size is limited to 32K per process due to the limited memory mapping
capabilities of the platform.

### Pentagon 1024K

A Russian clone of the Sinclair Spectrum with extensions. Early versions of
the 1024K Pentagon are a bit different, but probably work with the Scorpion
port.

Older Pentagon systems do not support mapping RAM into the low 16K so are
not currently supported.

Supported Features
- Keyboard
- Video
- NemoIDE
- Joystick (Kempston or Fuller or both)
- Kempston Mouse

Not Supported
- BetaDisk

### P112

A Z180 system with up to 1MB of memory and capable of running at 24.5MHz.

https://661.org/p112/

Supported Features
- Onboard serial
- 1MB RAM (required currently)
- SMC floppy controller
- Real time clock
- G-IDE IDE card
- 16550A option

Unsupported Features
- SCSI option

FUZIX can be booted on this system from CP/M or as a bootable floppy
disk. The file system then lives on an IDE or CF disk.

### Scorpion 256K

A Russian clone of the Sinclair Spectrum with extensions, but not quite the
same ones as the Pentagon.

Supported Features
- Keyboard
- Video
- NemoIDE
- Joystick (Kempston or Fuller or both)
- Kempston Mouse

Not Supported
- BetaDisk
- SMUC

### SAM Coupe

One of the very late 8bit machines the SAM Coupe was designed to be mostly
ZX Spectrum compatible but with its own additional features. It has a very
bizarre 32K banked memory model. The port is currently very basic but does
work.

Supported Features
- Keyboard
- Video
- AtomLite IDE
- SAMBus RTC
- Printer

Not Supported
- AtomIDE (original 16bit)
- Floppy disk (except to boot)
- Mouse (in test)
- MegaRAM
- SID
- Trinity

### Sinclair ZX Spectrum 128K / +2 with DivIDE or DivMMC / ZX-Uno

There are several ports for versions of the Sinclair systems and their
clones. This port supports the 128K spectrum and the gray +2 with 128K of
RAM and a DivIDE or DivMMC adapter. It will also run on the black +2 and the
+3 systems but they have their own port.

On the ZX-Uno this port knows how to to turn off the video contention
emulation and set the CPU up to 14MHz.

Program size is limited to 32K per process due to the limited memory mapping
capabilities of the platform.

Supported Features
- Keyboard
- Video
- DivIDE
- DivMMC
- Joystick (Kempston or Fuller or both)
- Kempston Mouse

### Sinclair ZX Spectrum +2 (black) / +3

This port supports the later CP/M capable +2 and +3 systems. A suitable hard
disk adapter is needed not just floppy drives. Unlike the 128K systems full
size processes can be run.

Supported Features
- Keyboard
- Video
- DivIDE
- ZXMMC
- Floppy Drive
- Joystick (Kempston or Fuller or both)
- Kempston Mouse

### Sinclair / Timex TC2068 / TS2068

Variants of the Spectrum systems for the US market and also in Argentia,
Portugal and Poland. At a software level it is not compatible with the
Sinclair systems but the hardware is except for some extensions. On this
platform FUZIX needs to be a cartridge.

Supported Features
- Keyboard
- Video (including extended Timex modes)
- DivIDE (via twister, must be set not to boot)
- Joysticks

Not Supported
- AY-3-8912 Sound

### Tandy COCO3

The Tandy COCO3 is supported by its own kernel as it differs in many ways
from the earlier systems.

Supported Features
- Keyboard
- Video
- Drivewire
- COCOSDC
- COCO on a chip SDC
- Glennside style IDE
- Printer

Not supported
- COCO3 timer/FIR based serial

### Tandy TRS80 Model I or III / LNW-80 / Video Genie / Dick Smith System 80 / PMZ 80

The classic TRS80 systems and various clones thereof. These systems are not
usable in FUZIX without add on memory banking cards. The current kernel
supports either the Alpha SuperMem (or compatibles) or the Selector add in.
Adding support for other mappers should not be too difficult.

For the Video Genie style systems the S100 based expansion unit and a
suitable S100 banked memory card can emulate the Alpha SuperMem behaviour
sufficiently or an new mapper could be added.

Program sizes are limited to 32K by the mappers.

Supported Features (Tandy style)
- Keyboard
- Video (Not LNW80 extensions)
- Floppy Disk
- Printer
- RS232 Interface (26-1145 or similar)
- Hard Disk (26-1132 or modern clones)
- Lower Case Kit (26-1104 or simple mod)
- Percom Doubler
- Holmes style speed up board (anything using port 254 bit 0)
- RTC (only for clock sync right now)
- Lo-tech IDE CF
- Alpha Joystick

Supported Features (Video Genie style)
- Keyboard
- Video
- Lower case (built in or add in)
- X-4010 Expansion
- EG3014 / X-4020 Expansion
- EG3016 Printer
- EG3020 RS232
- EG3022 S100 Adapter (but no card drivers)
- Percom doubler
- Tandy style RTC
- Lo-tech IDE CF
- In theory most things for the TRS80 via the convertor cable.
- Alpha Joystick

Unsupported
- Colour Genie (different system entirely), Genie II or later
- Le Guepard
- LOBO MAX-80
- LNW80 extended video modes
- Sound cards
- Tandy doubler
- Using Video Genie style printer or serial on TRS80 and vice versa

Tested On 0.2:
- Yes, emulation only.

### TRS80 Model 4/4D/4P with 128K+ RAM

The first TRS80 that can in theory run FUZIX out of the box as supplied.
128K of RAM is required. A 64K machine cannot run FUZIX.

Program size is limited to 32K per application due to the limits of the
memory banking.

Supported
- Keyboard
- Video
- Floppy Disk
- Hard Disk
- Huffman style banked memory on port 0x94
- Alpha Joystick

Not Supported
- Alpha Technologies SuperMem
- Anitek HyperMem
- XLR8R

Tested On 0.2:
- Yes, emulation only. Some problems to resolve on actual hardware.

## Retrobrew And RC2014 Systems

These are modern systems designed to be home built using classic 8bit
processors, or in some cases early 32bit ones such as the 68000. There are
two main groups of systems. The Retrobrew (formerly N8VEM) designs generally use
an ECB bus. The RC2014 systems are mostly a simpler to build design based
upon a minimalist Z80 bus with their origins in Grant Searle's homebrew CP/M
systems. There is increasingly an overlap between the two communities so the
divides are not always clear.

For more information on RC2014 see https://rc2014.co.uk. For more information on
Retrobrew systems see http://www.retrobrewcomputers.org.

### Easy-Z80

Easy-Z80 is an RC2014 compatible system with onboard SIO, CTC, ROM and RAM
and including memory banking.

https://github.com/skiselev/easy_z80

Supported Features
- Dual SIO serial ports
- CTC for timers and baud rate setting
- Some RC2014 add in cards

The Easy-Z80 supports Z80 interrupt mode 2, and any add in cards with
interrupt support need to be wired for IM2 support.

Tested on 0.2
- No

### LiNC80 SBC1

The LiNC80 is a single board computer kit that includes Z80 SIO, CTC and PIO
as well as having optional bank switched RAM and ROM. The port requires you
have wired yourself up at least one extra 16K RAM bank expansion.

http://linc.no/products/linc80-sbc1/

Supported Features
- SIO/2 serial ports
- CTC for SIO and clock
- PIO for SD card interface
- Compact Flash card

Tested on 0.2
- No

### N8VEM (now Retrobrew) Mark IV SBC

A Z180 based SBC with optional ECB bus interface.

https://www.retrobrewcomputers.org/doku.php?id=boards:sbc:z180_mark_iv:z180_mark_iv

Supported Features
- RS232 serial port
- RS422 serial port
- IDE/CF adapter
- SD card interface
- Onboard RTC
- Optional PropIO V2 adapter

Tested on 0.2
- Yes, real system

### SC108 and SC114

The SC108 is an RC2014 compatible CPU board with 128K of RAM and 32K of ROM
that holds the SCM monitor. The two RAM banks are switched as a full 64K
with only the ROM able to initially move data between banks. This makes it
tricky platform to work with, but FUZIX does support this model.

The SC114 is a closely related system with integrated motherboard and three
slots. It also has a bigbang serial port (not supported).

In order to run FUZIX on this system you must have an RC2014 IDE CF adapter
at 0x10, an SC104 or RC2014 SIO card at 0x80 (ACIA is untested) and an SC102
CTC at 0x88. The CTC must be jumpered from ZT2 to CT3.

It is possible to run with an RTC instead in tickless mode but this is not
recommended.

https://smallcomputercentral.wordpress.com/sc108-z80-processor-rc2014/
https://smallcomputercentral.wordpress.com/sc114-documentation/

Supported Features
- SC108 or SC114 CPU card
- 128K banked RAM
- SIO/2 dual serial
- Second SIO/2 at 0x84
- Z80 CTC with baud rate setting for serial port 2
- RC2014 RTC (optional)

Unsupported
- Bitbang serial port
- Customising baud rate clocks and control

Due to the limited memory the SC108/SC114 systems are not recommended for
multi-user usage. It is also not possible to use these boards with a banked
ROM/RAM card.

Installation

FUZIX is supplied as a raw filesystem image or CF card. Simply insert the CF
card and go. The default image does not enable cursor key editing. If you
want this then change your shell to /bin/fsh.

It is possible to enable both ports for login but due to lack of memory this
is not recommended.

Tested On 0.3-rc2
- SC114 with SC102 and SC104

### SC111

The SC111 is an RC2014 compatible Z180 system that can either be run with
conventional banked memory cards and I/O cards as a faster CPU, or with a
linear memory card in full Z180 mode.

https://smallcomputercentral.wordpress.com/sc111-z180-cpu-module-rc2014/

Supported Features
- Onboard RS232 ports
- IDE/CF adapter
- Optional RTC
- SC119 or similar ROM/512K RAM linear memory card (55ns RAM or faster)
- SCM monitor (ROMWBW is not yet supported on this hardware)
- SD card via CSIO and Z80 PIO port A bit 0 (CS)

The port only supports running in Z180 mode. If you are running with the Z80
cards then see the RC2014 port, but as with other RC2014 systems at this
time running in Z80 style banked mode does not include support for the Z180
serial ports and CSIO.

FUZIX is supplied as a raw filesystem image or CF card. Simply insert the CF
card and go. The default image does not enable cursor key editing. If you
want this then change your shell to /bin/fsh.

It is possible to enable both ports for login.

Tested on 0.3rc1
- Yes, real system

### Simple80

A glueless Z80 system with 128K of banked RAM in two banks along with 64K
ROM. The serial I/O is partially decoded between 0x00-0x7F, and the optional
CF adapter decodes 0x80-0xBF although a standard RC2014 adapter can also be
used (decodes at 0x10-0x17/0x90-0x97).

Supported Features
- Onboard SIO/2 ports
- IDE CF adapter (Simple80 or RC2014)
- Z80 CTC (eg SC102) at 0xD0
- RC2014 RTC (optional) at 0xC0

It is possible to run with an RTC instead in tickless mode but this is not
recommended. Note that the CTC requires a non-standard address due to the
partial decodes of the Simple80 devices.

Due to the limited memory the Simple80 systems are not recommended for
multi-user usage. It is also not possible to use these boards with a banked
ROM/RAM card.

Tested on 0.3rc1
- Yes, emulation

### Tiny68K

A 68000 based SBC with CF interface and a lot of memory.

https://www.retrobrewcomputers.org/doku.php?id=boards:sbc:tiny68k

Supported Features
- RS232 serial port
- IDE/CF adapter

Unsupported
- RTC on Tiny68K v2

Installation

FUZIX is supplied as a raw filesystem image or CF card. Insert the CF card
and type 'bo' at the boot monitor. It will then boot FUZIX instead of CP/M.
The current environment mostly matches the 8bit systems although much more
powerful commands and tools can be run.

Note that the IDE interface on the Tiny68K is byteswapped. FUZIX therefore
expects byteswapped media.

#### Important

Version 1 of the Tiny68K board has a reliability problem with some CF cards.
See the Tiny68K web site for details of workarounds and reworks.

### Tom's SBC (modified)

Tom's SBC is a modernized version of the Grant Searle's classic CP/M system.
FUZIX supports running on Tom's SBC with some minor modifications to allow
use of the full 128K RAM and an external timer source.

https://easyeda.com/peabody1929/CPM_Z80_Board_REV_B_copy-76313012f79945d3b8b9d3047368abf7

Supported Features
- RS232 serial ports
- IDE/CF adapter

Installation

FUZIX is supplied as a raw file system image or CF card. Insert the CF card
and type 'X' (boot CP/M) and then 'Y' to trigger the boot. The ROM will then
load the FUZIX loader instead and FUZIX will run. A ROM image with the bank
switch modifications is required.

Tested on 0.3rc1
- Yes, emulation only.

### Z80-MBC2

A Z80 system with an ATMega acting as the I/O subsystem and providing a
message based interface. The current ATMega firmware has some problematic
limitations but hopefully these will get fixed.

https://hackaday.io/project/159973-z80-mbc2-4ics-homemade-z80-computer

Supported Features
- RS232 serial port
- Virtual disk interface
- Real time clock module (required)

Program sizes are limited to 32K by the memory mapping arrangement of the
machine.

Installation

FUZIX is supplied as a bootable image and an 8MB disk image. Other disk
images can also be added. FUZIX looks for but does not require partition
tables on the images.

Tested on 0.3rc1:
- Yes, emulation only.

### Z80 Membership Card

The Z80 membership card is a set of cards that are designed to fit into an
altoids tin and provide a variety of classic Z80 environments.  The Z80
membership card is supported only in a full (3 card) configuration
with 512K RAM and SD card bitbang interface.

Supported Features
- UART serial port
- Bitbang SD card interface

Unsupported Features
- Hex keypad
- Seven segment display
- Bitbang serial port

Installation

FUZIX is supplied as a .HEX file that goes onto the FAT partition used by
the CP/M layers. The filesystem needs to go into its own partition as for
speed and compatibility FUZIX avoids the virtual disk interfaces and drives
the SD card directly.

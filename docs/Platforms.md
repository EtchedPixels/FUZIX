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
  may need additioanl work)

Tested On 0.2
- NC100: No
- NC200: Yes, real system

### Dragon 32 / 64 / Tandy COCO2

There are three different ports to differing configurations of this system

The first port is the coco2cart port. This requires a 64K Dragon or 64K Tandy COCO
with a Cloud 9 compatible IDE or COCOSD cartridge and Fuzix installed on the cartridge and disk.
Fuzix boots from cartridge, loads the rest of itself into memory and runs in
single process at a time mode. Due to lack of resources this port is quite
minimally featured but has some space for further improvement.

The second port is the dragon32-nx port. This supports the Dragon 32/64 or
Tandy COCO with at least 32K of memory and with Tormod's Spinx cartridge
giving 512K of banked RAM and an SD card slot. This port offers a full
featured Fuzix but with userspace limited to 32K per application. On 6809
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

### Sinclair ZX Spectrum 128K / +2 with DivIDE or DivMMC

There are several ports for versions of the Sinclair systems and their
clones. This port supports the 128K spectrum and the gray +2 with 128K of
RAM and a DivIDE or DivMMC adapter. It will also run on the black +2 and the
+3 systems but they have their own port.

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
platform Fuzix needs to be a cartridge.

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
usable in Fuzix without add on memory banking cards. The current kernel
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
- LNW80 extended video modes
- Sound cards
- Tandy doubler
- Using Video Genie style printer or serial on TRS80 and vice versa

Tested On 0.2:
- Yes, emulation only

### TRS80 Model 4/4D/4P with 128K+ RAM

The first TRS80 that can in theory run Fuzix out of the box as supplied.
128K of RAM is required. A 64K machine cannot run Fuzix.

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
- Yes, emulation only. Some problems to resulve on actual hardware.

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
have wired yourself up at least one extra 16K RAM RAM bank expansion.

Supported Features
- SIO/2 serial ports
- CTC for SIO and clock
- PIO for SD card interface
- Compact Flash card

Tested on 0.2
- No

### N8VEM (now Retrobrew) Mark IV SBC

A Z180 based SBC with optional ECB bus interface.

Supported Features
- RS232 serial port
- RS422 serial port
- IDE/CF adapter
- SD card interface
- Onboard RTC
- Optional PropIO V2 adapter

Tested on 0.2
- Yes, real system

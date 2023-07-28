# This is Fuzix for the RC2014 and related machines with 512/512K RAM/ROM

Modified for RC2014 with FlashROM/Ram board by Scott Baker <smbaker@gmail.com>.

Heavily based on prior work by Will Sowerbutts <will@sowerbutts.com>, 
Sergey Kiselev <skiselev@gmail.com>, Scott Baker <smbaker@gmail.com> and others.

## Supported Hardware

### Processor Cards

-	RC2014 compatible Z80 CPU card
-	RCBus compatible Z180 CPU card (40 or 80pin bus). rcbus-z180 is the recommended port for flat memory.

### Memory

-	512K ROM / RAM board. Note that some third party boards are partial clones or totally incompatible paging schemes and will not work.

### Serial Options

A RomWBW supported serial is required for boot by default
-	SIO/2 at 0x80 (optional CTC for speed setting)
-	68B50 (or emulations) at 0x80 (narrow decode required)
-	16x50 at 0xA0
-	Z180 onchip serial
-	Z80 KIO at 0x80 (also provides timer and SD)

Other serial options
-	Second SIO/2 at 0x84
-	16x50 ports at 0xA8/C0/C8/D0/D8/E0/E8/F0/F8 (*)
-	QUART at 0xBA

### Video/Keyboard Options

#### Video
-	EF9345 80 column text/graphics
-	TMS9918A 40 column text/graphics
-	Propeller Graphics (also as text console)

#### Keyboard / Mouse
-	PS/2 bitbang at 0xBB
-	PS/2 intelligent controllwer at 0x60/4
-	ZXKey at 0xFF

#### Joysticks
-	GPIO ports at 0x01 / 0x02

#### Clock/Timer
-	CTC with channel 2 and 3 jumpered to chain
-	TMS9918A
-	Z80 KIO
-	DS1302 clock at 0x0C or 0xC0 (time only no interrupt)

### Storage
-	CF adapter at 0x10 (with DMA option)
-	IDE PPA at 0x20 (no DMA)
-	EPP ZIP drive (via MG014 printer port)
-	Floppy controller
-	SD card over Z80 PIO. GPIO or KIO
-	Arduino SD card shield on Gluino

### Networking
-	Wiznet 5100 or 5300 module on carrier

### Other
-	Z80 DMA at 0x04
-	Console switches at 0xFF (read only)
-	Bus extender (see below)
-	I2C (early test)
-	Printing via MG014

## Unsupported
-	AMD9511A FPU. Some code is present but not yet finished
-	Sound cards (work in progress for future release)
-	Z180 in with flat memory card (use rcbus-z180)
-	SC108/114/Z80SBC64/SC7xx with different banking models see other targets intead.
-	SC110 CTC/serial (does not appear to be able to chain CTC pairs, as is neeed for IM1 mode). You can run a wire by hand. Can still be used for serial baud control.
-	Picked Dog 128/128K and 512/512K boards (different memory paging model to the standard RC2014)

## I/O Port Settings For Fuzix

0x00		SC129 GPIO
0x01		Joystick 0
0x02		Joystick 1
0x03		Free
0x04		Z80DMA
0x05		Free
0x06-0x07	I2C
[0x08-0x0B	Shared memory interface - user only]
0x0C-0x0F	82C55 Printer Port (MG014)	|	DS1302 (Z180)
0x10-0x17	IDE CF
[0x18-0x1F	Second IDE CF (not supported)]
0x20-0x23	IDE PPA
[0x24-0x27	Second PPA (not supported)]
0x28-0x2D	Wiznet module
0x30-0x3F	Free
(provisionally 30-37 SPIMaster)
0x40-0x41	Maccasoft Prop Graphics
0x42-0x43	AMD9511A FPU
0x44-0x47	EF9345
0x48-0x4D	Floppy controller (EPFDC)
0x48-0x58	Floppy controller (RC2014 *)
0x59-0x5B	Free (0x4E-0x5B free EPFDC)
0x5C-0x5F	TFT Panel0x60		PS/2				|	Z80PIO
0x61-0x63	Free				|	Z80PIO
0x64		PS/2				|	Z80PIO
0x65-0x67	Free				|	Z80PIO
0x68-0x6B	Z80 PIO/Gluino (and SD card)
0x6C-0x6F	Z80 second PIO			| 	Z80 PIO2 suggested
0x70-0x7F	MMU
0x80-0x83	Z80 SIO				|	KIO
0x84-0x87	Z80 SIO				|	KIO
0x88-0x8B	Z80 CTC				|	KIO
0x8C-0x8F	Free
0x90-0x97	IDE CF on EIPC and EasyZ80	|	Older CF mirrors here
0x98-0x99	TMS9918A/38/58
0x9A-0x9B	TMS9938/58
0x9C-0x9F	Free
0xA0-0xA7	16x50 Serial			|	SC26C92 serial       |    ACIA
		Also used sometimes for sound (0xA0-0xA2)
0xA8-0xAF	Free				|	SC26C92 serial (ctd)
[0xB0-0xB7	Analog Sticks - TODO]
0xB8		Bus extender
0xB9		Free
0xBA		QUART serial
0xBB		PS/2 Bitbang
0xBC		Z80 Coprocessor
0xBD-0xBF	Free
0xC0		DS1302 clock			|	Z180 Internal I/O     |    DS12885
0xC0-0xFF	Scanned for extra 16x50		|	Z180 Internal I/O
0xD0		AY-3-8910 (TODO)		|
0xD8		AY-3-8910 (TODO)		|
0xED						|	Also used by Z512
0xFE		ZXKey shadow
0xFF		ZXkey (W) Console switches (R)

Alternatives
0x80	KIO
0x00	Z84C15
0xC0-FF	Z180 internal I/O

(*) FDC uses 48/50/51/58


## RC2014 Extreme

This build uses the bus extender and moves the following. Config.h must be
edited and the kernel rebuilt for this feature set.

Joystick 1 and 2		01-02B8
I2C				06B8/07B8
MG ZIP/Lpr			0C-0FB8
Wiznet 5300			28-2DB8
PropGFX Video			40-43B8
EF9345				44B8/46B8
PS/2				60B8,64B8
I2C will move 			6C-6FB8
Analog sticks will move		B0-B7B8
RTC				C08B
Sound (AY-3-8910, TODO)		D0/D8B8
KIO				C0-DF (on main bus clocked by CPU clock)

## Setting It Up

Fuzix on the RC2014 expects a normal PC style compact flash card. Fdisk up the
card leaving the low space free as fdisk tries to do. Place a file system on
it (type 0x7E). Do not at this point place a swap partition on it (0x7F)
although you can certainly reserve on with a different type code.

The loader and attached kernel image needs to be written to blocks 2+.

"make diskimage" will do all the work to generate a file system, CF card image
and emulator image in Images/rc2014/.

As ucp and fsck.fuzix support offsets in the format path:offset you can access
the first file system with ucp emu-ide.img:1049600 if it starts at block 2048
of the emulated CF card. (use 1048576 for a real CF)

Then you can run

```` rc2014 -b -r RCZ80_std.rom  -i emu-ide.cf  -s -w -R -c

or for a KIO system

```` rc2014 -b -k -r RCZ80_kio.rom -i emu-ide.cf -R

or for a PPIDE setup

```` rc2014 -b -k -r RCZ80_std.rom -I emu-ide.cf -R

or for a TinyZ80

```` rc2014 -m tinyz80 -r /tmp/EZZ80_tz80.rom -i emu-ide.img

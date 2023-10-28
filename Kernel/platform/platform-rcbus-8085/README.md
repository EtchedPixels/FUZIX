# RCBUS 8085

Fuzix on the RCbus 8085 CPU/MMU + 16550A UART card setup

## Supported Hardware
- 	Etched Pixels 80C85/MMU card with CPU and MMU
-	512K linear RAM/ROM card (Small computer central SC119, Tom Szolyga, Marten Feldtmann, Rotten Snow #68) etc. *NOT* the Z80 banked memory card.
-	A 16Cx50 based serial adapter at 0xC0
-	PPIDE at 0x20

A clock source. Either the 82C54 card at 0x3C or a TMS9918A with
interrupt enabled at 0x98/0x99. If you are using the rev3 or earlier
board note the erratum and use a diode not a jumper for the IRQ

## Options
-	RCBUS DS1302 card at 0x0C
-	ACIA at 0xA0
-	CH375
-	NCR SCSI	(in progress)
-	Joystick at 0x01/0x02

## To Enable
-	TMS9918A as console maybe ? (what to do for keyboard ? - PS2)
-	TMS9918A extended features?

## Unsupported
-	Z80 support chip based devices (SIO, CTC, PIO, DMA)
-	IDE CF adapter (unreliable especially with higher CPU speed)
-	RCBUS Banked 512K RAM/ROM card
-	Hardware that uses the full 16bit I/O address space

For hardware requiring the standard RCBUS clock it is possible to
run with an 8MHz Tundra 80C85-8 part and a crystal at double the
classic speed (14.745Mhz)


## RCbus 8085 Addresses for I/O

0x0C		DS1302
0x20-0x23	PPIDE
0x3C-0x3F	82C54
0x70-0x7F	RAM/ROM card banking control
0x98-0x99	TMS9918A
0xA0-0xA7	ACIA (to add)
0xC0-0xC7	16550A UART

## Memory Map

### Kernel
0000-DFFF	Kernel (banked)
E000-FFFF	Kernel common (banked per process)

### User
0000-DFFF	User process (banked)
E000-FFFF	Kernel common (banked per process)

## To Build

Use cc85 from the Fuzix compiler kit

make diskimage will produce an IDE disk image (40MB mostly empty) and a matching
emulator disk image (emu-ide.img).

If you are building it yourself or updating the bootloader from the ROM package
goes on block 0 (with partition table), the kernel goes on blocks 1+ with the
file system and PC partition tables as normal.

The emulator image can be run with

rcbus-8085 -1 -r 512-8085.rom -B -I emu-ide.img -R -T

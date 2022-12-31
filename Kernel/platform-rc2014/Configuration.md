# Configuring the RC2014 for FUZIX

## Hardware

FUZIX hardware requirements are broadly similar to those of ROMWBW CP/M
except that a timer tick of some kind is required. Hardware that sits on the
RC2014 rx/tx bus lines and has no driver requirement should just work.

###  Minimum Requirements

* RC2014 Z80 CPU board
* 512K ROM/RAM board
* Serial interface (SIO at 0x80 or ACIA at 0xA0)
* CF adapter or PPIDE
* Timer source (CTC, TMS9918A, or QUART UART)

or

* RC2014 mini
* 512K ROM/RAM board
* CF adapter
* Timer source (CTC, TMS9918A, or QUART UART)

### Optional Items
* TMS9918A VDP graphics (at 0x98/0x99)
* QUART quad UART (at 0xBA)
* 16C550A UART (at 0xC0/8/D0/8... etc)
* ZXKey keyboard interface at 0xFE
* PS/2 keyboard/mouse/joystick/sound at 0xBB
* SD card via Z80 PIO at 0x68
* Z80 DMA at 0x04
* Z80 PIO at 0x60 and 0x68 (including Gluino)
* SC129 at 0x00
* Console switches at 0xFF (unless using ZXKey)
* RTC at 0xC0 or 0x0C

### Planned Items

* AY-3-8910 + Joystick

### Not Supported

* SBC64 (see rc2014-sbc64 port)
* Simple80 (see Simple80 port - needs corrected hardware)
* Z80 cards with 128K on board banked RAM (See SC108/SC114 port)
* Z80 with 64K RAM and paged rom (see rc2014-tiny port)
* Z180 linear memory systems (see SC111 and SC126 port)
* Z280RC (different port to be done)

### Planned Ports

* Z280RC

## Configuration

Configure the base hardware as required for ROMWBW CP/M and ensure ROMWBW is
running nicely.

If you are using a CTC then jumper the CTC so that ZT0 drives CT1. Wire ZT1 to CT3. If you are using it as
your system clock jumper it as shown in the manual to provide CLK from the on
board crystal and to provide that to CT0. Jumper CT2 to provide CLK2 for the second SIO2 port.

FUZIX uses timer 0 and counter 1 to divide the clock from 7.3MHz down to 50Hz,
counter 3 is then used to count the number of 50Hz ticks since the last time
they were processed. This allows the OS to keep good time when interrupts
are delayed for example by the floppy driver. Timer 2 is used to drive the
second serial port baud rate. (The odd order is needed as timer 3 has no
ZT output to clock something else).

Set the address to 0x88

## Interrupt Configuration

Fuzix does not use IM2 vectored interrupts due to the rather variable
support for them and the lack of standard connections for it.

## Installation

You can either use the provided Fuzix image on a suitably sized compact
flash card or you can build your own image. 

Fuzix will look for a standard (ie PC style) partition table on the CF card.
Each Fuzix partition is limited to 32MB, and 15 of them may be present.
This is way more than are needed for an 8bit system. They can be located
anywhere on the disk within the normal LBA range (LBA48 is not supported).

In future Fuzix will use the first partition marked with the type code 0x7F as
swap if present. 

Modern ROMWBW also uses partition tables so this works nicely. Early ROMWBW
(predating RC2014) has a 256 sector boot area at the start so it works there
too.

You can fdisk the media under Linux to provide a CP/M partition, one or more
Linux partitions  and then dd the file system image onto the desired partition.
The Fuzix bootable image can be copied onto a CP/M partition on the media
and run from CP/M, or installed in the boot area (TBD).

## ROMWBW

Note that beyond boot Fuzix does not use the ROMWBW services because
unfortunately at this point they are inadequate for its needs. In particular
ROMWBW does not support the native paging modes of the RC2014 ROMWBW board,
nor does it support interrupts. There are other areas of concern such as the
the floppy disk driver and Z80DMA.

If ROMWBW becomes usable then this may change but for now just be aware that
if you have a customized set up then you may need to customize Fuzix as well
as ROMWBW


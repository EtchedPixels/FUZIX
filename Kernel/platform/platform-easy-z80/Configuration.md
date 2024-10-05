# Configuring the Easy-Z80 for FUZIX

## Hardware

FUZIX hardware requirements are broadly similar to those of ROMWBW CP/M

### Minimum Requirements

* Easy-Z80 Z80 CPU board
* CF adapter

### Optional Items

* RC2014 DS1302 RTC

### Planned Items

* AY-3-8910 + Joystick
* VDP Graphics
* Z80 PIO (Joystick, Printer, SD card etc)

## Configuration

Configure the base hardware as required for ROMWBW CP/M and ensure ROMWBW is
running nicely.

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
the floppy disk driver.

If ROMWBW becomes usable then this may change but for now just be aware that
if you have a customized set up then you may need to customize Fuzix as well
as ROMWBW


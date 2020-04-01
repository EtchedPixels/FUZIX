# Platforms Under Development

## Classic Systems

### Amstrad PCW8256

Currently able to boot, to load init from floppy and to get as far as
forking. The fork code needs reworking and support for the IDE adapter
adding both to it and probably to the Joyce emulator.

Status: On hold.

### Apple IIe/IIc

Initial work to scope out the port and what would be needed. Loaders and
many other pieces of code will be needed before anything useful can be done.

### Atari ST

Kernel loader and kernel boot code is written basic video and keyboard works.
Currently working on the minimal device drivers and DMA support to get the
floppy disk and ASCI hard disk interfaces up and running

Status: Under development.

### Cromemco System III

Ran under emulation at one point. One big challenge is that the emulation does
not cover hard disks, and there appears to be no useful documentation on the
hard disk controller.

Status: On hold.

### MSX1

This is sufficient to boot and run on a system with FUZIX in a flat 64K
cartridge. Only the Sunrise IDE interface is supported. RAM must not be
in subslots or the system will crash.

Status: under development.

### Nascom

Scoping and design work for 80bus systems with Nascom style memory banking
and disk controllers.

### Pentagon

Scoping and initial work only. Needs support for ROM only in the low 16K
which needs future syscall changes.

### PX4 Plus

Abandoned for now.

### SPX302

Support for the modified SPX302, 68302 based platform from Bill Shen. The
kernel work is complete but the bootloader is proving problematic.

Status: under development.

## Retro Systems

### TBBlue

Initial sketches for the ZX Spectrum Next.

Status: on hold until the final platform becomes set in stone

### TGL6502

Port to an experimental 6502 emulating system, now discontinued.

Status: abandoned

### Z280RC

Z280 port to Bill Shen's Z280RC system.

Status: On hold.

### ZX Spectrum Uno

Port to use all the pieces of the ZXUno including the 512pixel screen,
combined Spectrum and Timex banking etc.

Status: In progress (you can use the existing zxdiv port)

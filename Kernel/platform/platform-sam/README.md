# Fuzix for the Sam Coupe

## Suppported Hardware

- Sam Coupe (256K/512K)
- Floppy drive (boot only at this point)
- Atomlite IDE
- Printer Port

## Options
- Mouse
- SAMBus RTC

## Installation

make diskimage produces fuzix.mgt a boot floppy image, disk.img a raw hard disk
image and fuzix.hdf an emulator disk image. Boot from the floppy with F9 or BOOT.

On the emulator set the fuzix.mgt as floppy 0, set up AtomIDE lite with the
fuzix.hdf file, reset and then hit keypad-9 (F9 for the machine)


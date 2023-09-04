# ZX Spectrum +2A/+3

## Hardware

This port supports the ZX Spectrum +2A/+3 with either a zxmmc or a simple 8bit
IDE interface. It boots from floppy in +2A/+3 mode although there is no
fundamental reason another boot process wouldn't be possible.

## Building

````make diskimage

Unlike the other ZX ports this one does not need ESX or other firmware on
the disk media but stands along using the floppy boot.

## Emulation

FUSE is capable of running this in +3 mode with the zxmmc. For some reason
using the Simple 8bit IDE interface fails

## Real Hardware

Write the image to a disk, boot from it using the ROM option.


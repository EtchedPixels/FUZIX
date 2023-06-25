# TRS80 Model 4/4D/4P

## Base Systems

- Tandy TRS80 Model 4 (including 4D and gate array models) 128K
- Tandy TRS80 Model 4P 128K

## Supported Hardware

- Floppy Disk (required for boot at this point)
- Hard Disk (Tandy or compatible including things like FreHD)
- Alpha Technology Style Joystick
- Tandy Hi-res card (graphics use only)
- Huffman style memory banking on port 0x94

## To Do

- 4P and modified rom model 4 hard disk boot
- Microlabs Grafyx reporting for graphics use only
- Alpha technology supermem
- Anitek MegaMem as a ramdisk/swap
- Anitek Hypermem
- FreHD specific features
- Split base 128K kernel from a thunked kernel for the memory bank cards

## Unsupported

- XLR8R/4ccelerator - really deserves its own Z180 mode port
- M3SE except as far as compatibility goes

## Installation

make diskimage
Set up hard1-0 as a hard disk image on a FreHD
Put the relevant boot.jv3 disk on the FreHD and use the FredHD apps to make
a floppy disk of it.

## Emulator Notes

Copy boot.jv3 to disk4p-0 or disk4-0 and use hard4p-0 from the emulator with xtrs or a
FredHD.

You will still need a boot floppy but you can make those using a FreHD or
other tools from the JV3

# Genie IIs / Speedmaster

## Status

Work in progress - not yet functional

## Supported Systems

- Genie IIs with one or more 192K memory expansion cards

## Supported Hardware
- On board video at 64x16
- Keyboard
- Floppy disk (very basic)
- Tandy compatible hard disk at 0xC8
- SASI at 0x00

## Unsupported

- Bitmap graphics, bitmap 80x25 mode
- German additional keyboard keys

## Installation

make diskimage

This will product a boot disk image and a hard disk image.

## Emulator Notes

Copy boot.jv3 to disk1-0 and use hard1-0 from the emulator with xtrs or a
FredHD. Copy boot.jv3 to disk3-0 and hard1-0 to hard3-0 for a model 3
emulation.

You will still need a boot floppy but you can make those using a FreHD or
other tools from the JV3

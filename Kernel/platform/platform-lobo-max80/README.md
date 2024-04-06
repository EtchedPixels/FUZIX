# LOBO MAX-80

## Supported Hardware

Lobo Max 80
- Keyboard
- Video
- Serial
- Beeper

SASI hard disk

## To Do

- 8" boot media
- SASI boot
- Floppy Disk Driver
- UVC (if enough info exists)
- Preload font in bootup
- Expansion bus I/O

## Unsupported

- Probably UVC will end up this way

## Installation

make diskimage gets you a 5.25" SS/DD boot floppy and a raw hard disk image

## Emulator Notes

max80 -A boot.5 -S disk.img

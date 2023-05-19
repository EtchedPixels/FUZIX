# Fuzix for Lazer VZ200

## Supported Hardware

- Laser VZ200
- SD Loader
- Optional video expansion mod

## Installation

make diskimage produces a disk.img file which is a raw CF image and a choice
of an sdboot.rom or loader.vz file. Only the sdboot.rom is tested as such.
There are also the pieces for booting it from the official ROM but this also
needs work

## Emulation

z300 -2 -s disk.img -R sdboot.rom

Add -a for the video RAM expansion mod


# Z80ALL

Bill Shen's 25.175MHz VGA and PS/2 capable Z80 Machine.

## Memory map

Kernel
0000-7FFF	Kernel
8000-FFFF	Kernel (fixed)

User
0000-7DFF	Process
7E00-7FFF	Udata stash

## Hardware:

Currently this build supports
- Onboard video
- PS/2 port
- CF interface
- Timer tick

## TODO

- Allow for a second console with F1/F2 switch
- Support the Quad serial 16x50 @0xC0 (and maybe KIO ?)

## Installation

make diskimage

boot it. It does not need or want a softloaded ROMWBW.

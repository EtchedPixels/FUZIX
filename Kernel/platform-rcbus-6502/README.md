# RCBUS Platform with 6502/65C02/65C816 proceessor

Basic support for a 6502 RCBUS banked memory platform.

Use a modern cc65 from https://github.com/cc65/cc65 (2.18 as shipped in
many Linux distributions is just fine).

## Supported Hardware

- RCBUS 65(C)02 processor card with I/O at $FExx
- RCBUS 65C816 processor card with I/O at $FExx
- 512K/512K banked memory card
- 16C550A compatible UART at $FEC0
- 6522 VIA at $%FE60
- RC2014 IDE adapter at $FE10

At the moment the support is fairly basic. Other devices could easily be
added but have not yet been.

To run the 512/512K card at 7.3MHz requires using some 74AHC parts,
otherwise 3.58MHz is recommended.

There is currently no swap support.

## Installation

Burn the ROM from the RCBUS ROM archive into a 512K flash if you don't already
have it on the system

make diskimage

copy disk.img onto a CF adapter. The current image is about 40MB (mostly empty)
and can be written to anything that size or larger.

Insert CF card into system and power up.

If you see just a letter 'R' on the console at 38400 baud then something is
wrong with the system set up. If it is working you will get a copyright
message and a set of debug as it loads from CF.

Emulator:

Build the ROM image as before
Copy emu-ide.img somewhere

./rcbus-6502 -1 -r 6502.rom -i emu-ide.img

Add -R to enable an emulation of the RCBUS RTC card, add -w for the
WizNet 830MJ.

# Fuzix for 68008 RCBUS

## Required Hardware
- 80pin extended backplane
- 68008 (if prototype card with wire fixes)
- RCBUS PPIDE card
- 512K linear RAM/ROM
- 16x50 serial at 0xC0
- v0.03 68008 ROM

## Optional Hardware
- DS1302 RTC at 0x0C
- Joysticks at 0x01 / 0x02
- SC129 GPIO at 0x00
- Console switches at 0xFF

At the moment a 16x50 is expected at 0xC0 as support for the 26C92 based
companion card has not yet been tested in the system or emulator.

## Untested but code now present for:
- QUART
- 26C92 including SD card via SPI
- TMS9918A at 0x98 (work needed for graphics due to I/O blocking)
- PS/2 Keyboard/Mouse
- ZXKey

## Installation

make diskimage produces a bootable CF image and an emu-ide.img for the
emulator.

Run the emulator with

./rcbus-68008 -r 68krom -p 68008dos.cf -f -R

2:
BOOT

Not everything will work correctly as there is no timer wired up yet. Thus
shutdown and anything doing time delays will fail. To shutdown instead do

	remount / ro
	halt


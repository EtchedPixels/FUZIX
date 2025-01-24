# Fuzix for the ZX Spectrum SE (Chloe) and ZX Uno

## Hardware

This port supports systems with a DIVMMC interface coupled with combined
128K and Timex MMU/Video. On the ZX Uno make sure the Timex MMU option is
enabled.

The platform currently enables the following devices
- Keyboard
- Video
- DIVMMC
- Fuller/Kempston joystick
- Kempston mouse

## Installation

To load we use the ESXDOS extension that is generally found with most
DivMMC and DivIDE interfaces. Copy FUZIX.BIN into / on the ESXDOS drive and
copy FUZIX into BIN. Then boot from ESXDOS with .fuzix

## Emulator

make diskimage
Boot from an ESXDOS enabled DIVMMC emulation

For Fuse use the Spectrum SE emulation. Zesarux will not run this code as it
has various ZX Uno and DIVMMC emulation bugs.

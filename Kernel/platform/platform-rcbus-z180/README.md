# Fuzix for RCbus style Z180

## Supported Hardware

- Z180 CPU card with 512K/512K flat memory card
- SC126 SBC
- SC130 SBC
- SC131 SBC

Various other compatible setups like the Z80.no #25 and #70a should also
work. An 18.432MHz base clock is assumed.

## Not Supported

- SCM Monitor Firmware (for SC111 see sc111 target)
- RIZ180 (see riz180 target)

## Options

- RC2014 CF adapter at 0x10
- DS1302 RTC at 0x0C
- 126fix SPI multiplexor
- SD card on CSIO (SC126 and similar)
- Wiznet 5200 card
- Wiznet 5500 on CSIO
- SC126 I2C
- SC126 1MB RAM mod

## Installation

make diskimage

The resulting disk.img is a raw CF image.

Boot with RomWBW firmware.

For emulation

rcbus-z180 -m sc126 -i emu-ide.img

The emulator supports using the CSIO SPI via a Bus Pirate so you can attach
real SPI devices to the emulation

## Configuration Options

- CONFIG_NET_W5200: set instead of W5500 if using a 5200 card
- CONFIG_NET_W5500_FDM: set if you have a W5500 but no chip select line
- CONFIG_DEV_I2C: set to enable SC126 I2C

## Boot options

- "turbo" - experimental double speed option
- "wiznet" - force probing for WizNet 5500

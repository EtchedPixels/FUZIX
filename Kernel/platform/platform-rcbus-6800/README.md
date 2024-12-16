# RCBUS 6800/6808

## Supported Hardware

- RCBUS 6808 CPU card
- CF Disk Adapter
- 512/512K RAM card
- 16x50A UART
- 6821/6840 I/O card


## Installation

make diskimage
write the image to a CF adapter then boot using the 6800 Boot ROM

## Emulation

make diskimage
rcbus-6800 -b -i emu-ide.img -1

## Things To Do

- Switch to tinydisk
- Swap support
- Stop using CONFIG_SMALL
- Serial speed/format support


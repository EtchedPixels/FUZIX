# MB020

## Supported Hardware

MB020, a 68020 motherboard with flat memory and rcbus slot. A 16x50 UART is
currently required by this port. ACIA really needs to be added.

## Options

None.

## Installation

make diskimage

For emulation test with mb020 -1 -r bootrom.rom -i emu-ide.img

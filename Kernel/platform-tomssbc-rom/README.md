# Fuzix for Tom's SBC version C in 64K EPROM

## Hardware
This is a port for Tom's SBC with a 64K EPROM (eraseable strongly recommended)
The only modification is to bend up DCDA on the SIO-2 and wire it to a
8Hz square wave source. The board as designed has no timer source as CP/M
2.2 doesn't actually need one. There are plenty of ways to make an 8Hz
square wave circuit, including these days just using one of the tiny PIC
devices to count it.

(Other frequencies will do but I happen to own an 8Hz source). Set the
clock frequency in config.h and fiddle as needed with devtty.c as the
kernel needs to see a clock divisible by 10 so some fudging is done).

## Memory map

````
Kernel

0000-00FF	Vectors (present in all banks)
0100-3FFF	Kernel (four banks)
4000-BFFF	User space
C000-FFFF	All kernel data common and stacks

User

0000-00FF	Vectors (present in both banks)
0100-BFFF	User space
C000-FFFF	Kernel data command and stacks
```

## To Do
- Push the data and common up higher
- Maybe support the CP/M emulator
- Swap only needed blocks for speed up

The ideal case of 128K RAM + banked ROM kernel would need a banked thunked
Z80 setup which we don't yet support and would be a little bit too
interesting right now!

## Emulation

````
make diskimage
searle -r fuzix.rom -T -i emu-ide.img -t
````

Real Hardware

Burn the 64K ROM image and fit the EEPROM or Flash

````
make diskimage
````

Write the disk image to CF

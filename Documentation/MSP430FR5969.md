# Building Fuxiz for the MSP430FR5969

## Introduction

The MSP430FR5969 is a very small embedded microcontroller made by Texas
Instruments. See here for more information:

  http://www.ti.com/product/msp430fr5969

You can run Fuzix on it, if you connect up an SD card reader to the
appropriate pins. The easiest way to do this is to get the Launchpad
evaluation board:

  http://www.ti.com/tool/msp-exp430fr5969

...and then get the SD card Boosterpack from here:

  http://store.43oh.com/The-CardReader-SDCard-BoosterPack

...and plug it in. The Fuzix port's console is, by default, on the right
pins so that the Launchpad's USB serial emulation will Just Work. Power up
the board, plug in the SD card, start your serial terminal, hit the reset
button, and you should get a shell.


## What's supported

The MSP430FR5969 does not have a lot of RAM. Fuzix will run a single 23kB
user process at a time, and uses swap onto the SD card to allow you to run
other programs. Timeslicing is not supported. You can run up to four
processes (although it'd be trivial to add more).

There's a full filesystem on the SD card with a tonne of software. The Bourne
shell runs, most basic commands are present, and you can program in sh or
forth. It's actually surprisingly snappy and pleasant to work with.

Even the games work.


## Required software

This has only really been tried on Debian Linux, on an amd64 machine. If you're
on something else... sorry. It shouldn't be too hard to build on any Unixoids.

You will need:

  *  TI's port of gcc; you can get that from here:
     http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSPGCC/latest/index_FDS.html

  *  A copy of mspdebug, for programming the board. I used the version pckaged
     in Debian. You will, unfortunately, *also* need the tilib driver for it,
	 which is a pain to build. You can get it here. Good luck.
	 http://www.ti.com/tool/mspds

  *  A blank SD card with at least 20MB (!) of capacity.


## Building

Once you've got all that set up, you should just be able to do:

  make -j PLATFORM=msp430fr5969

...and it'll all happen. You'll end up with three files; the kernel image,
called kernel-msp430fr5969.elf, a corresponding map file which you can ignore,
and the file system image, called filesystem-msp430fr5969.img.

Now you're ready to install it!


## Installation

First, prepare the SD card. Create an MSDOS partition table, and add two
partitions at least 10MB in size. The partition type or active flag is
irrelevant.

The first one is the file system; dd the filesystem image onto it. The second
is the swap partition, which doesn't need preparation.

Insert the SD card into the device and connect it up. Start your serial
terminal.

Now flash the kernel onto the device with:

  mspdebug tilib "prog kernel-msp430fr5969.elf"

You'll see the kernel image being written into the device's persistent
storage, and then it'll reset, and (with luck) you'll get a shell.


## Problems

It's a bit fragile. Some applications crash it, such as wumpus. I don't know
why; there should be enough RAM. 

There's something wrong with the SD card interface, and it's rather
unreliable. Once the SD card is detected correctly, it seems to be fine. I
think it's not being initialised correctly.

It's really, really short of RAM. Ideally the whole kernel needs to be
rejigged so that the kernel is built in large mode, which means a lot of it
can live in high memory, which isn't accessible to user processes (which run
in small mode). However, Fuzix currently assumes that
sizeof(int) == sizeof(void*), so that may not work. Also, large mode
instructions on the MSP430 are way bigger than small mode ones, and the extra
overhead may not be worth it. If only mspgcc supported mixed mode, with 20-bit
code pointers and 16-bit data pointers...


David Given
dg@cowlark.com


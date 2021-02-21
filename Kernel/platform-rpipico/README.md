# The Raspberry Pi Pico port

## Introduction

The Raspberry Pi Pico is a very small and very cheap microcontroller board
based around the Raspberry Pi Foundation's RP2040 chip. It's got two Cortex-M0+
cores, 2MB of onboard NAND flash which can be used for code via a demand-paging
system, and 269kB of RAM.

The Fuzix port runs in single-tasking mode, using an SD card connected via the
second SPI interface for the root filesystem and swap.

## Configuration

Out of the box:

  - /dev/hda is the SD card. /dev/hda1 needs to be at least 2MB and is the swap
    partition, and /dev/hdb2 is the root filesystem.

Connect the SD card to the following pins:

        Pico pin     RP2040 pin    SD card pin
        --------------------------------------
           16           12            MISO
           17           13             CS
           19           14            SCK
           20           15            MOSI

Remember to also connect the SD card's GND to any ESP8266 GND pin and Vcc to
3.3V. Not 5V, or it won't work.

The console is UART0, and runs at 115200 baud.

![Wiring diagram](doc/wiring.jpg)

## Building and installation

You need to install the [Raspberry Pi Pico SDK](https://www.raspberrypi.org/documentation/pico/getting-started/).

```
cd Kernel/platform-rpipico
vi Makefile
# At this point, you need to edit the Makefile to tell it where the Raspberry Pi Pico SDK lives.
make world -j
./update-flash.sh
```

You should now end up with `build/fuzix.uf2` and `filesystem.img`. The `uf2`
file can be flashed onto the Pico in the usual way (i.e. connect it up as a
mass storage device and copy the file on). Alternatively, you can use OpenOCD
to load `build/fuzix.elf`.

To create the SD card, take a blank card and do this (ignoring everything after
the `#`):

```
$ CARD=/dev/sdz # change this to whereever your card is
$ sudo fdisk $CARD
o         # creates a new DOS partition table
n         # create partition
p         # primary
<return>  # default partition number, 1
<return>  # default first sector
+2M       # partition size
n         # create partition
p         # primary
<return>  # default partition number, 2
<return>  # default first sector
+32M      # partition size
w         # save partition table
$ sudo dd if=filesystem.img of=${CARD}2 bs=1M
```

The card should now be ready to use. Insert it into the card reader and power
cycle the Pico; you should see the boot messages appear on the serial console.
When prompted for the date and time, just hit RETURN. Once you get to the login
prompt, log in as root (no password).

The first thing you probably want to do is `stty erase '^?'` to make the DELETE
key in your terminal work properly. There are also some games in `/usr/games`.

Important notes:

  - this will destroy whatever's on the card. Make sure you don't misstype the
	device name or you could destroy what's on your hard disk.
  - if your card is a `/dev/mmcblk0` device, that last line should use
	`of=/dev/mmcblk0p2` instead of `of=/dev/sdz2`.

## Userland

The Pico Fuzix port runs generic Cortex M0 ELF pie binaries, which are
theoretically portable to other ARM or Cortex devices, but this hasn't been
tried yet. There's 64kB available for both code and data; the system call
interface uses `svc`.

The Pico's mask ROM contains many useful routines which can be used instead of
libgcc, which would reduce the binary size. This hasn't been done yet because
it would render the binaries non-portable.

## Using the NAND flash

The Pico's built-in NAND flash is supported, but not terribly useful as you
still need an SD card for swap. It appears as `/dev/hda` inside Fuzix (the SD
card is `/dev/hdb`). It's mapped via the Dhara FTL library, so you get proper
wear levelling, but it requires formatting before use. To do this, from inside
Fuzix do:

```
$ blkdiscard /dev/hda
$ mkfs -f /dev/hda 32 2746
```

The FTL library requires empty flash sectors to work efficiently; the Fuzix
filesystem has trim support, so the FTL library gets notified when sectors
become free, but the flash needs initialising first. `blkdiscard` erases the
entire flash; `mkfs -f` prevents `mkfs` from writing zeroes to the entire
filesystem. If you let it do this, then the FTL library will think every block
will be in use and will work extraordinarily badly.

## Issues

There are many.

  - userland binaries can't find their error messages.
  - CPU exceptions should be mapped to signals.
  - single-tasking mode should be switched off (which would allow pipes to
    work).

...and probably others.

## Postscript

dg@cowlark.com


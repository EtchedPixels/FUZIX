

## Introduction

The ESP8266 is a very small and very cheap microcontroller and WiFi interface
by Espressif Systems. It was originally intended as a turnkey WiFi module which
spoke AT protocol via a UART, but it actually turns out to be a rather capable
microcontroller in its own right.

It's based around a LX106 32-bit CPU (one of the Xtensa LX6 family) with 96kB
of data RAM, 64 kB of code RAM, and an external NAND flash connected via SPI.
A rather cunning demand-paging system allows the flash to be memory mapped,
using 32kB of the code RAM for a cache. The ESP8285 is a very similar device
except that it has an internal SPI flash that runs in a different mode.

The Fuzix port runs in multi-tasking mode with the root filesystem on the
internal NAND flash. An SD card can be connected to the second SPI interface
for further file storage and swap.

## Building

```
make TARGET=esp866 kernel diskimage
```

You need the `xtensa-lx106-elf` gcc toolchain --- install the
`gcc-xtensa-lx106` package from Debian.

## Configuration

The base system assumes a typical ESP8266 configuration with a 26MHz
peripheral clock and the CPU usually running at 80MHz. If you get weird
serial speeds run esptool and set the peripheral clock in config.h to twice
the clock it reports.

The flash mappings and clocking is managed by the hardware based upon the
image header. These are set by the esptool when you flash the image. The
'-ff' option sets the SPI flash frequency and the -fm the mode. 40 (Mhz) is
a safe flash setting but most boards can do 80. The ESP8266 boards generally
support 'qio' (the fastest option), some require 'dio' and the ESP8285 requires
'dout'. You can adjust these in the command or the make burn settings. If it
fails try more conservative settings. Many of the bigger ESP-12 MCUs (often
used for NodeMCU require dio).

Out of the box:

  - /dev/hda is the NAND flash. The system is configured for a 2MB flash chip,
	which gives you a root filesystem of 1279kB.
  - /dev/hdb is the SD card. This is only probed on startup --- you can't hot
    swap it.

Connect the SD card to the following pins:

        NodeMCU pin      GPIO pin     SD card pin
        -----------------------------------------
            D5             14            SCK
            D6             12            MISO
            D7             13            MOSI
            D2              4            CS

Remember to also connect the SD card's GND to any ESP8266 GND pin and Vcc to
3.3V. Not 5V, or it won't work (and that's likely to be a permanent change).

The console is UART0, and runs at 115200 baud.

Doing `make -C Kernel/platform-esp8266 burn` will flash the kernel onto the device
connected on `/dev/ttyUSB0` with this command:

    `esptool --port /dev/ttyUSB0 write_flash 0x00000 image.elf-0x00000.bin 0x10000 image.elf-0x10000.bin -ff 80m -fm dio'

Doing `make -C Kernel/platform-esp8266 fburn` will flash the filesystem onto
the device in the same manner.

## Swap

Out of the box, Fuzix runs in swapless mode. This gives enough memory to run
most normal programs (you can use `free` to see how much you have left). If you
want more, you can enable swapping to the SD card.

To do this, create a 2048kB (4096 blocks) partition on the SD card.  Then use
the `swapon` command to enable swap. You can see swap usage with `free`.

```
# fdisk -l
                      START                  END
Device    Boot  Head Sector Cylinder   Head Sector Cylinder  Type  Sector count

/dev/hdb1        33      3        0     38      6        1    83          4096
/dev/hdb2        38      7        1     58      8       18    83         65536
# swapon /dev/hdb1 4096
# free
         total         used         free
Mem:       160           56          104
Swap:     2048            0         2048
# 
```

You can't turn swap off again.

You probably can swap to the NAND flash, but it's a terrible idea.

## Pin Mappings We Use

GPIO1           Serial TX       }       To the USB UART on the NodeMCU kits
GPIO3           Serial RX       }

GPIO4           SDcard CS       }
GPIO5           Second CS       }
GPIO12          MISO            }       SD card interface (note Arduino
GPIO13          MOSI            }       tends to use GPIO15 not 4 but GPIO15
GPIO14          SCK             }       is also used during boot)

GPIO6-11        SPI Flash

GPIO0           Bootloader button
GPIO2           Usually an on board LED, used for W5500 reset
GPIO9/10        Used in QIO Flash mode, otherwise not used
GPIO16          Deep sleep wakeup, not used

A0              Not used

## Memory Map

````

3FFE8000-3FFFBFFF       Data RAM
        3FFE8000-3FFF7FFF       User space
        3FFF8000-3FFFBFFF       Kernel
3FFFC000-3FFFFFFF       ETS data RAM
        3FFFC000-3FFFC713       Unused (move stacks and tmp buffers?)
        3FFFC714-3FFFC733       Firmware flash structure we need to keep alive
        3FFFC734-3FFFFFFF       Buffers
40000000-4000FFFF       Internal ROM (some routines  usable)
40100000-40107FFF       Instruction RAM (64K). Fast memory for programs
        40100000                        - Bootstrap (bottom - overwritten)
        40100000-40107DFF               - Userspace code
        40107E00-40107FFF               - NAND flash lowlevel functions
		40108000-4010BFFF               - Cache block #1 (used by us for userspace code)
		4010C000-4010FFFF               - Cache block #2 (used for SPI flash mapping)
40200000-402FFFFF       SPI Flash (cacheable), only accessible as if it was iram
        40200000                - Bootloader
        40201000                - Kernel
        40220000                - NAND flash fs

````

## Userland

ESP8266 binaries are specific to this platform and won't run on any other ESP
modules: the Xtensa architecture is very flexible and ABI compatibility is too
hard. The binaries use the Fuzix tiny architecture and are limited to 64kB of
data and 31.5kB of code. They're not relocatable.

The ESP8266's mask ROM contains many useful routines which can be used instead
of libgcc, which reduces the binary size. These aren't documented, but you
might find this ROM disassembly useful:

        http://cholla.mmto.org/esp8266/bootrom/

## Issues

There are many.

  - someone needs to overhaul the SD SPI code who understands it.
  - not all the ROM routines are hooked up to userland binaries.
  - the tensilica asm could do with a clean up by someone who knows it better

...and probably others.

## Author

dg@cowlark.com did the original port.

## Building Without Debian

Get gcc 10 and currentish binutils from the FSF mirrors and unpack them.
Get the Debian source package from https://packages.debian.org/unstable/gcc-xtensa-lx106
Unpack it and apply the three patches from local-patches in the archive to
the gcc you unpacked. Copy the xtensa-config.h from overlay/include over
the one in gcc.

For binutils get https://packages.debian.org/unstable/binutils-xtensa-lx106
and copy overlay/bfd/xtensa-modules.c into the bfd directory of binutils.
Copy the xtensa-config.h over the one in the binutils include directory.


````

% mkdir /opt/esp8266
% mkdir binutils
% cd binutils
% ../binutils-2.37/configure --prefix=/opt/esp8266 --target=xtensa-lx106-elf
% make
% make install
% PATH=/opt/esp8266/bin:$PATH
% mkdir gcc
% cd gcc
% ../gcc-10.3.0/configure --target=xtensa-lx106-elf --prefix=/opt/esp8266 --disable-decimal-float --disable-libffi --disable-libgomp --disable-libmudflap --disable-libquadmath --disable-libquadmath-support --disable-libssp --disable-libstdcxx-pch --disable-libstdc++-v3 --disable-nls --disable-shared --disable-threads --disable-lts --enable-lto --enable-target-optspace --disable-_cxa_atexit --without-long-double-128 --disable-multilib --enable-cxx-flags=-fno-exceptions --with-gnu-as --with-gnu-ld --with-headers=no --without-newlib --without-included-gettext --enable-languages=c,c++
% make
% make install

````

You now have an ESP8266 compiler set.

## TODO

- Use some of the ROM library routines that don't themselves meddle with static
  data. Make a kernel set and a user set (RBOOT has a good list) Note that we
  can't use many of them as they are not instruction fault aware.
- Move to 'parent runs first' - needs some tricky changes in the platform fork
  code (see the Z80 examples)
- Clean up the low level asm code - too much duplication, things saved that are
  not needed. Could do with a clean up by someone who speaks Tensilica LX106
  properly.
- Can we autodetect and manage stuff like peripheral clocks ? (apparently it
  hides in the phy structures at the end of the flash?)
- Shared ROM libc
- XIP ROM app code ?

vim: set ts=4 sw=4 et


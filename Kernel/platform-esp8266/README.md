# The ESP8266 port

## Introduction

The ESP8266 is a very small and very cheap microcontroller and WiFi interface
by Espressif Systems. It was originally intended as a turnkey WiFi module which
spoke AT protocol via a UART, but it actually turns out to be a rather capable
microcontroller in its own right.

It's based around a LX106 32-bit CPU (one of the Xtensa LX6 family) with 96kB
of data RAM, 64 kB of code RAM, and a built-in (usually) 4MB NAND flash
connected via SPI. A rather cunning demand-paging system allows the flash to be
memory mapped, using 32kB of the code RAM for a cache.

The Fuzix port runs in single-tasking mode, using an SD card connected via the
second SPI interface for the root filesystem and swap. The NAND flash is
usable, and is mapped via a FTL to provide wear levelling, but is too slow to
produce a useful interactive system.

## Building

```
cd Kernel
make TARGET=esp8266
```

You need the `xtensa-lx106-elf` gcc toolchain --- install the
`gcc-xtensa-lx106` package from Debian.

## Configuration

Out of the box:

  - /dev/hda is the NAND flash. You get about 2.5MB of usable space.
  - /dev/hdb is the SD card. /dev/hdb1 needs to be at least 2MB and is the swap
	partition, and /dev/hdb2 is the root filesystem.

Connect the SD card to the following pins:

        NodeMCU pin      GPIO pin     SD card pin
        -----------------------------------------
            D5             14            SCK
            D6             12            MISO
            D7             13            MOSI
            D2              4            CS

Remember to also connect the SD card's GND to any ESP8266 GND pin and Vcc to
3.3V. Not 5V, or it won't work.

The console is UART0, and runs at 115200 baud.

Doing `make -C platform-esp8266 burn` will flash the kernel onto the device
connected on `/dev/ttyUSB0` with this command:

    `esptool --port /dev/ttyUSB0 write_flash 0x00000 image.elf-0x00000.bin 0x10000 image.elf-0x10000.bin -ff 80m -fm dout`

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

  - userland binaries can't find their error messages.
  - CPU exceptions should be mapped to signals.
  - single-tasking mode should be switched off (which would allow pipes to
	work).
  - someone needs to overhaul the SD SPI code who understands it.
  - not all the ROM routines are hooked up to userland binaries.

...and probably others.

## Postscript

dg@cowlark.com

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

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

## Building

You need to install the [Raspberry Pi Pico SDK](https://www.raspberrypi.org/documentation/pico/getting-started/).

```
cd Kernel/platform-rpipico
vi Makefile
# At this point, you need to edit the Makefile to tell it where the Raspberry Pi Pico SDK lives.
make world -j
```

The `build/fuzix.uf2` file contains just the kernel, and can be flashed in the
normal way. It won't be much use without a filesystem.  There's a script called
`update-flash.sh` which will create a userland filesystem which can be dd'd
onto the SD card.

## Userland

The Pico Fuzix port runs generic Cortex M0 ELF pie binaries, which are
theoretically portable to other ARM or Cortex devices, but this hasn't been
tried yet. There's 64kB available for both code and data; the system call
interface uses `svc`.

The Pico's mask ROM contains many useful routines which can be used instead of
libgcc, which would reduce the binary size. This hasn't been done yet because
it would render the binaries non-portable.

## Issues

There are many.

  - userland binaries can't find their error messages.
  - CPU exceptions should be mapped to signals.
  - single-tasking mode should be switched off (which would allow pipes to
    work).
  - the NAND flash driver is broken and disabled.

...and probably others.

## Postscript

dg@cowlark.com


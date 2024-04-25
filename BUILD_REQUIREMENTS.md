# Build Requirements

This is a WIP list of the build requirements for Fuzix for diffferent
targets. At the moment lots of compilers are used according to the target.
The intent is to reduce this to gcc for bigger machines and the Fuzix native
compiler for the small ones.

## General

Bash shell
GNU Make
Byacc

## 6303/6800/6803

CC6303 head from https://github.com/EtchedPixels/CC6303

The compiler, tools and Fuzix are developed in parallel.

## 6502 / 65C816

Any vaguely modern cc65. The builds are currently tested with cc65 v2.18

## 6809

6809 gcc 4.6.4 with the lwtools patches and lwtools.

## 68HC11

GCC 3.4.6 built for 68HC11. This is about the last GCC before 68HC11 support
was dropped where it still actually works.

## 68000

*WARNING*: The Fedora gcc-m68k package appears to be completely broken.

In fact the gcc build environment appears to be a bit of a train-wreck for
M68K options. See [README.68000.md](README.68000.md) for a working example.

## 8080/8085

Fuzix Bintools
https://github.com/EtchedPixels/Fuzix-Bintools

make
make install

Fuzix C Compiler
https://github.com/EtchedPixels/Fuzix-Compiler-Kit

Build the assembler first and ensure it's on your path

make install

## 8086

gcc-8086

## ARM (PI Pico)

Raspberry Pi Pico SDK

## ARM (TM4C129X)

## ESP8266

xtensa-lf106-elf gcc tool chain. The build is tested with the Debian
gcc-xtensa-lx106 package.

esptool

## Rabbit 2000/3000

Dropped. We don't yet have a usable platform/emulator for this. Will come
back once we have assembler and compiler support from ccz80.

## Z80/Z180

Fuzix Bintools
https://github.com/EtchedPixels/Fuzix-Bintools

make
make install

Fuzix C Compiler
https://github.com/EtchedPixels/Fuzix-Compiler-Kit

Build the assembler first and ensure it's on your path

make install

For most kernels you still need a slightly modified SDCC from
https://github.com/EtchedPixels/sdcc280. For all but banked kernels the
current 4.x SDCC ought to work.

This will gradually change.

## eZ80

Temporarily dropped. Will come back when the new C compiler has ez80 support
and we have an actual meaningful ez80 platform for development (or a usable
emulation).

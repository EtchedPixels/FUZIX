# Status of 0.5 Work

## Move 8080 and 8085 to Fuzix C Compiler

All targets completed. Userspace also moved. No relocatable binaries (as
before). Can build applications natively.

## Move Z80 To Fuzix C Compiler

Userspace entirely moved. Binaries relocatable as before. Need to move to
a better relocation solution for doing dynamic object modules.

Switched to the latest updates to the compiler and bintools kits. These put
everything in a sensibly laid out location.

### Completed

- Nascom
- RC2014-Tiny
- SBC2G
- SBCv2
- SC720
- Searle (and related)
- Simple80
- Tom's SBC (RAM based)
- Z1013
- Z80 MBC2
- Z80 Membership Card
- Z80Pack
- ZRC

### In Progress

- Zeta V2

### To Complete

Some of these need an assembly macro expander adding, others need banking
supported added to the compiler and linker. For now they rely on the
modified SDCC 3.8 for building the kernel

#### Needs macro expander
- 2063
- Ampro Littleboard
- Cromemco
- Easy Z80
- Linc80
- Micro80
- Z80Retro

#### Needs banking
- KC87
- Pentagon
- Pentagon 1024
- RC2014
- Scorpion
- Tom's SBC (ROM)
- TRS80 model I/III
- VZ200
- ZX Spectrum with DIVMMC/DIVIDE

#### Other
- Amstrad NC series
- CP/M 2.2
- Dyno
- EZRetro
- Genie EG64
- JeeRetro
- MSX1
- MSX2
- Memotech MTX
- N8
- P112
- PCW8256
- RBC Mark 4
- RCBUS SBC64
- RCBUS Z180
- Rhyophyre
- RIZ180
- SAM Coupe
- SC108
- SC111
- Scrumpel
- SmallZ80
- SocZ80
- TC2068
- TRS80 model 4
- ubee (Microbee)
- YAZ180
- Z180ITX
- ZX Spectrum +3
- ZX Uno

### Work In Progress

These platforms are unfinished experimental work anyway

- Adam (converted)
- C128 Z80
- Gemini
- Genie IIs
- VZ700
- Z280RC
- Z80 BIOS
- ZX Spectrum 48K with extended DIVMMC/IDE
- ZX Evolution
- ZX Spectrum with SpectraNet

# Target Status

Last updated 2024/07/04

## 2063

Builds, passes basic tests

## 68KNano

Passes basic tests.

## Adam

Work in progress, not targetted for 0.5

## AmproLb (Ampro Littleboard)

Passes basic tests.

## Amstrad NC100

Passes basic tests

## Amstrad NC200

Passes basic tests

## AppleIIe

Long term project - probably needs a better compiler. Not for 0.5

## Atari ST

Early 68K work. Now core 68K is stable can be resurrected. Probably not for
0.5

## C128-Z80

Early experiments, broke VICE so on hold

## Centurion

Early work in progress

## Challenger III

Builds, needs a 0.5 test run

## COCO2 (64K, no cartridge)

Builds, passes basic tests.

## COCO2Cart (64K with cartridge ROM)

Passes basic tests

## COCO3

Builds, passes basic tests

## CPM22

Experimental only.

## CROMEMCO

Passes basic tests

## Dragon (MOOH)

Passes basic tests

## Dragon (NX32)

Passes basic tests

## Dyno

Passes basic tests

## Easy-Z80

Passes basic tests

## ESP32

Early WIP only

## ESP8266

Builds, testing pending

## EZRetro

Builds, not tested

## Gemini

Early WIP, probably not for 0.5

## Geneve

Early WIP for TMS99xx. Needs compiler fixes and more yet

## Genie-EG64

Builds, passes basic tests

## IBMPC

Early sketches only

## JackRabbit

Early sketches only

## JeeRetro

Builds, not tested

## KC87

Builds, passes basic tests

## LINC80

Builds, passes basic tests

## LOBO-MAX 80

Builds, passes basic tests

## MB020 (Plasmo)

Builds, passes basic tests 

## Micro80 (Plasmo)

Builds, passes basic tests

## Mini11 (Etched Pixels)

Builds, passes basic tests

## MO6 (Thomson)

Work in progress only (need info on cartridge headers to progress)

## MSP430FR59

Retired in 0.2, bitrotted

## MSX1

Builds, passes basic tests

## MSX2

Builds, passes basic tests

## MTX (Memotech)

Builds, test pending

## Multicomp09

Builds, not tested

## N8 (Retrobrew)

Builds, passes basic tests

## NASCOM

Builds, passes basic tests

## OSI50x

WIP only

## P112

Builds, not tested

## P90MB (Plasmo)

Builds, passes basic tests

## PCW8256 (Amstrad)

Builds, passes basic tests

## PDP11

Work in progress - need a compiler that actually works (gcc still fails on
basic stuff alas)

## Pentagon

Builds, passes basic tests

## Pentagon 1024

Builds, passes basic tests

## Pico68K

Builds, passes basic tests

## PX4 plus (Epson)

Early WIP, probably never feasible

## PZ1

Builds, passes basic tests

## Rabbit 2000

To merge with jackrabbit

## RBC-Mark4 (Retrobrew)

Builds, passes basic tests

## RBC-Minim68k (Retrobrew)

Builds, passes basic tests

## RC2014

Builds, passes basic tests

## RC2014-Tiny

Builds, passes basic tests

## rcbus-1802

Compiler experimentation

## rcbus-6303

Builds, passes basic tests. Needs compiler from 2024/06/18 or later.

## rcbus-6502

Builds, passes basic tests

## rcbus-65C816

WIP compiler bring up

## rcbus-6800

WIP compiler bring up

## rcbus-68008

Builds, passes basic tests

## rcbus-6809

Builds, passes basic tests

## rcbus-68hc11

Builds, passes basic tests

## rcbus-8080

Builds, passes basic tests

## rcbus-8085

Builds, passes basic tests

## rcbus-80C188

Early WIP only

## rcbus-ns32k

Builds, passes basic tests

## rcbus-sbc64

Builds, passes basic tests

## rcbus-super8

Compiler bring up work

## rcbus-tms9995

Compiler bring up work

## rcbus-tp128

Builds, passes basic tests

## rcbus-z180

Builds, passes basic tests

## rcbus-z8

Early work, dev board needs changes

## rhyophyre

Builds, passes basic tests

## riz180 (Plasmo)

Builds, passes basic tests

## rpipico (Rapsberry Pi Pico0

Builds

## sam (Sam Coupe)

Builds, passes basic tests

## sbc08k

Builds, passes basic tests

## sbc2g

Builds, passes basic tests

## sbcv2

Builds, passes basic tests

## sc108 (Small Computer Central)

Builds, passes basic tests

## sc111 (Small Computer Central)

Builds, passes basic tests

## sc720 (Small Computer Central)

Builds, passes basic tests

## scorpion

Builds, passes basic tests

## scrumpel

Builds

## searle (Grant Searle Z80)

Builds, passes basic tests

## simple80 (Plasmo)

Builds, passes basic tests

## smallz80

Builds, passes basic tests

## socz80

Builds, test pending

## sorceror (Exidy)

Work in progress only

## T100

Early work in progress only

## TC2068 (Timex)

Builds, passes basic tests, requires Fuzix bintools 2024/6/18 or later.

## Tiny68K (Plasmo)

Builds, passes basic tests

## TM4C129X

Builds, test pending

## TO8 (Thomson) 

Builds, test pending

## TO9 (Thomson)

Work in progress, stretch goal for 0.5

## Toms SBC

Builds, passes basic tests

## Toms SBC (ROM)

Builds, passes basic tests

## TRS80 (Model 4, 4D, 4P)

Builds, passes basic tests

## TRS80m1 (Model 1/3)

Builds, passes basic tests

## ubee (Microbee)

Builds, passes basic tests

## v65c816(-big)

Work in progress only

## v8080

Builds, passes basic tests

## vrisc32

RISCV toolchain still breaks on us

## vz200

Builds, passes basic tests

## vz700

Experiment only, may well not be possible

## yaz180

Builds

## z1013 (Robotron)

Builds, passes basic tests

## z180itx (Etched Pixels)

Builds, passes basic tests

## z280rc

Work in progress, post 0.5

## z80all (Plasmo)

Builds, passes basic tests

## z80-bios

Experiment only

## z80-mbc2

Builds, usually passes basic tests, debugging a possible interrupt problem

## z80membership

Builds,passes basic tests

## z80pack

Builds, passes basic tests

## z80retro

Builds, test pending

## zeta-v2

Currently converting to new compiler

## zrc (Plasmo)

Builds, passes basic tests

## zx128

Obsolete experiment (128K spectrum and microdrive)

## ZX+3 (ZX Spectrum +3)

Builds, passes basic tests

## ZXDiv (ZX Spectrum 128K with DIVIDE/DIVMMC)

Builds, test pending

## ZXDiv48

Experiment only

## ZXEvo (Evolution)

Early WIP only

## ZX Spectra

Work in progress. 0.5 stretch goal

## ZX Uno

Builds, passes basic tests

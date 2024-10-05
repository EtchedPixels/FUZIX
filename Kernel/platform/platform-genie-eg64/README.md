# Fuzix for a Video Genie with EG 64.3 or TRS80 + Lubomir Soft Banker

## Supported Hardware

- Video Genie (aka Dick Smith System 80, PMC 80/81, TRZ-80)
- or TRS80 model 1
- Soft banker (EG 64.3 or Lubomir Soft Banker - both are the same interface)
- Expansion interface with RAM fitted
- Hard disk interface (or compatible modern retro such as the FreHD)

** Note: The EG 64.3 is required. The EG 64.1 does not have the needed banking **

## Options

- Floppy disk (pretty basic)
- Lower case (built in, mod or Tandy 26-1104)
- EG3016 printer interface
- EG3020 RS232 adapter (no interrupt so very limited)
- Tandy 26-1145 or similar RS232 (no interrupt so very limited)
- Percom style doubler
- Holmes speeed up board (or anything else using port FE bit 0)
- RTC at 0xB0
- Alpha Products style Joystick

## Unsupported
- Tandy double density kit
- Genie II/III series machines
- Supermem and other bigger bankers (See trs80m1 target)

## Installation

make diskimage will build a TRS80 disk image for a FreHD or emulator, along with a
boot disk in .jv3 format. These can be turned into real disks using the FreHD or
fed to an emulator with Lubomir softbanker support.

To test move boot.jv3 to disk1-0 and do

xtrs xtrs -lubomir -romfile videogenie.rom 


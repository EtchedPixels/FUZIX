# ZRC

A Z80 an CPLD based system that can plug into a RC2014 bus if desired

The system has 2MB of RAM but all of that is accessible only by paging the low
32K. The current build will only use the low 512K or so. It's not clear what
to do with the rest - as a ramdisc it's not really any performance difference
to CF.

One option might be to just reserve a bank for the udata stash and claim 512
bytes of user space back ?

## Memory map

Kernel
0000-7FFF	Kernel
8000-FFFF	Kernel (fixed)

User
0000-7DFF	Process
7E00-7FFF	Udata stash


## Hardware:

Currently this build supports

- Onboard not-quite ACIA including interrupt
- Onboard CF
- External Wifi
- DS1302 clock

It really needs a time source adding, perhaps also more serial devices
and a video/keyboard option.

Installation

make diskimage

boot it. It does not need or want a softloaded ROMWBW.

### To fix
- Linker dynamic libraries

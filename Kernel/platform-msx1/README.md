Idea - Fuzix in ROM


System constraints

4 slots, each maybe 4 subslots

Proposed memory map therefore is

Cartridge

0000-3FFF	Fuzix
4000-7FFF	Fuzix
8000-BFFF	Fuzix
C000-FFFF	Copy of common and other areas for C000-FFFF in RAM

Running

Kernel mode

0000-BFFF	Fuzix(Cartridge)
C000-FFFF	Kernel common and data

User mode

0000-BFFF	Process bank (with stubs in spare low bytes)
C000-FFFF	Kernel common and data

Difficulties
- Have to map 0x4000-7FFF to a device for some I/O devices so we can't
  always directly do I/O to user space. Kernel buffers are high so ok.
  Kernel I/O routines can live in other 16K chunks or common.

- We have to load the cartridge header at 0x4000 or 0x8000 - neither is at
  all convenient!

BIOS lives at 0:0

Disk catridges live at 4000-7FFF in some bank/sub-bank usually with their
ROM in the same 16K chunk

You can't move blocks up and down address spaces

Done
Add the tool to stuff the image properly into ROM for unpacking
Finish the basic catridge logic
Build the user map from the RAM map
Finish the ROM and RAM map detection logic
Disk I/O routines need bounce buffers for 4000-7FFF range due to the
limited mapping system.
Speed up all the ei/di mess by just keeping a private variable for irqoff
state
Get common and discard and initialized in C000-FFFF of the ROM and copied
out
Sunrise IDE support (except probing)
Swapping
Fast user copy routines

To Debug

(Hopefully fixed)Seem to have had some disk corruption - how to reproduce/why ?
Display flicker of weird chars when scroll/clear bottom line ?
We blow up with slot expanders early in boot (in find_ram). Seems to be a
problem if the cartridge is in an expander.

In Progress

Move the switch helper into both banks so we can fix the FIXME in map_kernel

To do

Detection logic by ROM hash and find the sunrise etc
Switch to inverse video cursor and inverse high font
Can we put find_ram etc in discard ?
Do we need a font and should it move somewhere to make const space ?
Tune swap logic so we only read/write relevant pages
We badly need the cached path walk or some other optimizations on user copy
or the bank switch
Remember current map for kernel/user so we can fast track map_save/restore
map_kernel etc by knowing if we are mapping k->k u->u or a transition and
we can label all other cases with a value meaning 'other' as we don't need
to worry about fastpaths that much
Given the MSX1 mess consider just writing JIT code for k-> u->k etc so non
stupid machines are not too slow. In particular if we see that RAM is not in
the same slot as anything else we use we can set the subslots once and just
bash 0xA8 like a sane computer.
For machines with bussed I/O do we want a /dev/iobus or similar that has the
info for a program to unpack (ROM hashes etc) ?

Target initial I/O devices

Carnivore2 and Sunrise IDE (I believe same logic)
MegaSD etc
MegaRAM (detect and use for swap for now)

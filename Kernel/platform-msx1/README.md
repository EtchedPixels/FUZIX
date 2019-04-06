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

In Progress

Move the switch helper into both banks so we can fix the FIXME in map_kernel
Work out how map/unmap an I/O device needs to work to be usable. Do we grab
the live map and just edit it for 4000-7FFF then set that map ? Do we
precompute and save kernel and user maps for the device ?
Sunrise IDE support

To do

Detection logic by ROM hash and find the sunrise etc
Sensible user copy routines: We know kernel data is always in common except
some awkward corner cases (constants) going K->U. So we can spot the to user
case of a 'low' source and bounce it or similar, while just doing a user
map and ldir for the others. We badly need the cached path walk though!
Remember current map for kernel/user so we can fast track map_save/restore
map_kernel etc by knowing if we are mapping k->k u->u or a transition and
we can label all other cases with a value meaning 'other' as we don't need
to worry about fastpaths that much

Target initial I/O devices

Carnivore2 and Sunrise IDE (I believe same logic)
MegaSD etc
MegaRAM (detect and use for swap for now)

Fuzix in a 64K ROM cartridge

Currently requires you have 64K of RAM and a Sunrise style IDE cartridge. It
should work with all memory mappings but some subslotted main memory cases
might be a bit iffy.

For MSX1 machines with an MSX2 mapper it would be far better to finish
teaching the MSX2 code about TM9918A video modes.



Cartridge

0000-3FFF	Fuzix
4000-7FFF	Fuzix
8000-BFFF	Fuzix
C000-FFFF	Copy of common and other areas for C000-FFFF in RAM

Running

Kernel mode

0000-BFFF	Fuzix(Cartridge)
	(sometimes 4000-7FFF is remapped for the I/O devices)
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

To Debug

We blow up with slot expanders early in boot (in find_ram). Seems to be a
problem if the cartridge is in an expander. (need low and high routines for
this ?) - may now be fixed ?

In Progress
Move the switch helper into both banks so we can fix the FIXME in map_kernel

To do
** Fix console size reporting on a change - general VT layer isuse **
Can we put find_ram etc in discard ?
Fast bank switch for machines without subslot madness, or when not doing
subslottery
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

DONE	-	Carnivore2 and Sunrise IDE (I believe same logic)
MegaSD etc
MegaRAM (detect and use for swap for now ?)

Generally these devices have mappers attached and are not pure SD so we
should address them by sorting out the MSX2 port.

Fractal2000 SD/Mapper/MegaRAM
SD512 style cards

Most of the SD cards are much the same
- bytwide SPI data port
- control register (CS pins)
- status of some kind (presence, wp)
- range of addresses to ldir data in and out (the joys of MSX MMIO)


SD512 is window 7B00-7EFF for I/O
7FF0 control
	1-0: card selected
		seems to be 0 = interface status mode
			1-2 card, 3 off ?
7FF0 status
	0: SD change detect
	1: SD present
	2: SD write protect
	Interface mode it gives
	0: 1 = RAM enabled
	1: 1 = Mapper mode 0 = megaram mode
7FF1 timer

The older MSX SD mapper and Fractal 2000 seem to be

4000-47FF SD_DATA
4800
	 0: present slot 0
	 1: presetn slot 1
	 2: wp 0
	 3: wp 1
	 4: ram enabled 0
	 5: mapper 1 megaram 0
	 7: 1 = spi busy
and writes with the chip selects on bits 1-0 (0 = low)
6001 write 1 to enable SPI

(need to revist our hash area maybe - if 4000-47FF is used by some for I/O
we will hash I/O spaces)

The MegaFlashRomSCC+SD style is a bit more complicated

7FFF
7-6: map mode
00	SCC
40	64K RAM
80 	ASC8
C0	ASC16
5: 0 = SCC 1 = normal mapper
3:	disable 4000-5FFF in Konami mode
2:	lock mapper register/disable it
1:	disable mapper
0:	enable 512K limit in SCC mapper / 256K in Konami

7FFD:	offset register
7FFE:	offset register to 2 bits (in 1-0)

7FFC:	config:
7:	lock/disable this regisdter
5:	disable SRAM (subslot 2)
4:	DSK mode - remaps banks 0-1 to DSK kernel
3:	set to map PSG to A0-A3 as well as the low ports
2:	disable subslots (no memory etc)
1:	protect flash
0:	enable flash

Subslot 2 when enabled is a mapper (IO FC-FF)
Subslot 3 is the MegaSD bit

4000-4FFF:	SD read/write (CS low)
5000-57FF:	SD read/write (CS high)
5800-5FFF	bit 0 selects SD bit 1 selects SD 2



Testing

If using an emulator

make diskimage
openmsx -carta fuzix.cart -ext ide -hda disk.img

You can pick an appropriate MSX 1 system such as the good old Toshiba HX-10
which is normally fairly easy to find on ebay, or you can also run with
CBIOS. In the CBIOS case you must have the Fuzix cartridge in slot A so that it
boots before the sunrise firmware (which will otherwise fail with
"CALLED NON EXISTANT BASIC ROM").

For the real hardware you need a 64K cartridge without mapper, and for
simplicity probably battery backed RAM. It ought to be possible to write
Fuzix to one of the modern cartridges in flash but I've not tried that
experiment.


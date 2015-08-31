First wild guess at an MSX1 target

And it is exactly that a guess 8)


No memory paging routines implemented yet so not very useful!


This will also almost certainly need udata copy on process switching at
least until someone fixes sdcc to generate bankable code. At that point we
may be able to bank the OS kernel in a cartridge.


Q: should we work to the following map


	0x0000-0x7FFF	User space / switchable with kernel
			(in theory if kernel code is cartridge
			 could we even use RAM + megaram?)
	0x8000-0xBFFF	Kernel (end of code, data, highcode, common)
	0xC000-0xFFFF

or do we bite the bullet about relocatable binaries (which we need to do
for the sinclair at least) and do

	0x0000-0x00FF	Vectors
	0x0100-????
	0x4000-0xBFFF	User space/switchable with kernel rest
	0xC000-0xFFFF

which would allow the use of more typs of megaram without LDIR tricks.

There are some other interesting tradeoffs on size handling if you make the
OS a cartridge as well as you can then use main memory via LDIR and lazy swap
the other 16K of app space for a 48K app so you only end up copying for
switches between big apps but get 48K apps. That might mean binaries at
0x4000-0xFFFF or 0x0000-0xBFFF, but either way means shunting the kernel
around in the pure RAM case to avoid the overlap (eg loading the kernel
at 0x4000 (with discard and bootstrap at 0x100))


Other mappers that might be found include

	- MSX2 mapper (in which case we should probably make the msx2
		platform detect the early type VDP and use that ?)

	- Zemina (requires LDIR copying stuff)
# FUZIX for Z80Pack Emulation

The kernel sets up Z80Pack with 60K banks and a 4K common for a 512K
machine. The actual emultion itself goes up to 16MB. The values can
be altered and this can be a useful test platform for various setups.

## Memory Map

Fuzix runs with the memory map as follows

Bank 0:
0000-0080	Vectors
0081-0084	Saved CP/M command info
0088		FUZIX kernel start
End of kernel:	Common >= 0xF000
		uarea
		uarea stack
		interrupt stack
		bank switching code
		user memory access copy
		bank copy buffer because Z80Pack switching is slow
		interrupt vectors and glue
		[Possibly future move the buffers up here to allow for more
		 disk and inode buffer ?]
FFFF		Hard end of kernel room

Bank 1 to Bank n:
0000		Vector copy
0080		free
0100		Application
ECFF		Application end
ED00-EFFF	uarea stash

## Building

make diskimage

This will produce a boot.dsk for the A drive of Z80Pack and an
drivep.dsk file which is a Z80Pack hard disk image (512MB) for drive P. 
Fuzix will use drive J as a swap device.

This has changed from the older builds that use drive I. A 4MB drive isn't
enough to hold the base system and all the compilers and tools so it was
moved to use 32MB of P drive.

## To build by hand:

Put the kernel at the end of a floppy image from cyl 60
Add the fs in the first 58 cyls (366 blocks)

Put the bootblock in sector 0

dd the kernel image to offset 193024

ie

dd if=fuzix.bin of=drivea.cpm bs=1 seek=193024 conv=notrunc


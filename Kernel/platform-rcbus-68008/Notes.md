Testing build for rcbus-68008

You need a disk image with the boot loader on LBA 0 and the kernel on LBA 1+
Block 0 should hold a partition table as well.

At the moment a 16x50 is expected at 0xC0 as support for the 26C92 based
companion card has not yet been tested in the system or emulator.

Untested but code now present for:
	QUART
	26C92 including SD card via SPI
	TMS9918A at 0x98 (work needed for graphics due to I/O blocking)
	PS/2 Keyboard/Mouse
	ZXKey

To add:
	Floppy ?
	TMS9918 graphics - can't access directly so need a helper something
	like

	[Count.B][Stream]

	where bit 7 of count indicates data/control and we also do minimal
	filtering on control writes


Memory Map:
00000 - 0FFFF		ROM
10000 - 1FFFF		I/O window (ROM in user mode)
20000 - 7FFFF		ROM
80000 - _end		Kernel image
_end  - ?????		Memory pool for user space
????? - FFFFF		Boot time buffer and memory allocations


To set up on a real CF/disk /dev/sdX.

*** Take care to get the right /dev/sd device, or you can make a very
*** nasty mess.

make
fdisk /dev/sdX
add a partition type 7E of size 32MB (65536 blocks)
optionally add a DOS/Windows partition (not NTFS and no long names)

dd if=Kernel/platform/loader of=/dev/sdX
dd if=Kernel/fuzix.bin of=/dev/sdX seek=1

This puts the partition table and boot loader on block 0 and the kernel on
blocks 1->128 or so of the 2048 that are left free by a modern fdisk for a
loader.

cd Standalone/filesystem-src
./build-filesystem -X /dev/sdX1 256 65535

Puts a valid file system on the Fuzix partition and all the needed files


For emulation the same applies but there is a 2 block header on the virtual
disk. The easiest approach is probably:

makedisk 4 68008dos.cf
dd if=68008dos.cf of=tmp skip=2
fdisk tmp
create a new 32MB primary partition at the default start (block 2048)
set the type to 0x7E
create a new DOS partition using the rest

dd if=Kernel/platform/loader of=foo conv=notrunc
dd if=Kernel/fuzix.bin of=foo seek=1 conv=notrunc

cd Standalone/filesystem-src
./build-filesystem -X fstmp 256 65535
dd if=fstmp of=tmp seek=2048 conv=notrunc

This builds the disk image in tmp. Now we put it back

dd if=tmp of=68008dos.cf seek=2 conv=notrunc


You can then run it with

./rcbus-68008 -r 68krom -p 68008dos.cf -f -R


Not everything will work correctly as there is no timer wired up yet. Thus
shutdown and anything doing time delays will fail. To shutdown instead do

	remount / ro
	halt

If your CF is big enough you can move an image from emulation to real CF by
doing dd if=68008dos.cf of=/dev/sdX skip=2 to remove the header.


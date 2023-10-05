# TRS80 Model 4/4P

(or right now more accurately sdltrs/xtrs)

## Emulator Bugs

Repeating instructions like LDIR appear to be misemulated. LDIR
is performed an emulated block copy, not as an iterated LDI. The
real processor actually implements LDIR as  "LDI, if not done
PC -= 2". FUZIX doesn't do any overlapped LDIR tricks so shouldn't
be affected.

Other bugs in RETI, IFF handling etc have been fixed in my xtrs
tree.

These fixes should now be in the current official xtrs.

## Requirements: 

128K or banked RAM fitted
Hard disk drive (will be used for swap), or a suitable memory
expander board could be used with a bit of tweaking (or both!)

## Memory Map:

Base memory 0-FFFF (with a fair bit of slack) is used for the kernel
User processes run 0-7FFF in bank U64L32 or U64U32, in both cases
with the upper 32K being kept as the kernel bank.

It would be good to support 64K processes using the bank32 model.
Before that can be done however the TRS80 will need a custom
user copy function to deal with access to the upper 32K by mapping
it low instead. It will also need the location of the uarea stash to
be binary size dependent. Swapper write out is also fairly hairy
for the same reasons. So for now we just handle a pair of 32K
processes.

Processes that don't fit are swapped to hard disk. Without swap you
can run a pair of 32K processes, just enough for stuff like
bootstrap.

## Adding Support For Other Banked RAM:

See trs80-map*s, and the various map_* functions. These can be extended
to use U_DATA__U_PAGE+1 to carry a second byte of data. The
main.c for the platform sets up the pagemaps as a 16bit value
which currently is just the opreg bits for the two banks.

map_save/restore will also need to handle any sub banking
arrangement.

Finally up MAX_BANKS in platform/config.h accordingly.

Setting It Up (xtrs from EtchedPixels github needed for some of the extra
memory emulations)

````
make diskimage

cp ??/Images/trs80/hdboot.jv3 disk4p-0
cp ??/Images/trs80/hard4p-0 hard4p-0

xtrs -model 4p
````

You can add on various memory expanders, bankers and graphics options to
taste. For example -huffman will give you the extreme maxed out 2MB version
of the Huffman memory expansion.

You will still need a boot floppy at this point but just boot with device
1. Swap is configured to be on the end of the hard disk

Banking Models

Currently Supported:
- TRS80 model 4 with inbuilt memory bank switching
- Port 0x94 modification to add extra internal RAM to a 4 and 4P

TODO
- Finish debugging floppy write
- Add floppy write for other sector sizes
- Debug other sector sizes
- Add floppy format

- Write a bootblock for floppy that loads the kernel from hard disk
- Add a 2 cylinder partition (64K) for the kernel with a new type
- Make the floppy blindly load from the right offset if it's hard
- to find it, if not find it and do that by type

# Notes for the Video Genie EG 64.3 / Lubomir Soft Banker

## The Hardware

The two have the same interface (and may well be the same logic). They add
an I/O port at 0xC0 which controls how memory is mapped

Port 0xC0

| Bit   | State | Meaning          |
|:------|:------|:------------------|
| Bit 7 | Clear | Writes to 0000-37DF are ROM|
|       | Set   | Writes to 0000-37DF are RAM|
| Bit 6 | Clear | Reads from 0000-37DF are ROM|
|       | Set   | Reads from 0000-37DF are RAM|
| Bit 5 | Clear | I/O is visible at 37E0-3FFF|
|       | Set   | RAM is visible at 37E0-3FFF|
| Bit 4 | Clear | High 32K is from RAM|
|       | Set   | High 32K is from the expansion box|

## Bugs

## To Add

The current code leaves the I/O mapped between 37E0 and 3FFF. It would be
possible to just map it when needed and claim back 2K of memory. This
would require small changes to the keyboard scan, the floppy and the
video output. It would also clean up all the juggling of things in memory
to fit round the hole.

Claiming the 2K back would let us add minimal video driver hooks for some of
the common add-on cards and UDG cards.

Can we do lower case on UDG card if not on base machine ?

## Memory Map

0000-37DF	Small parts of the kernel, video driver and data
4000-5FFF	Common, constants initialized data and buffers
6000-7FFF	Always user space (used for discardable code at boot)
8000-FFFF	Banked kernel or user

## Notes

Note that there appears to have been a different 'EG MBA' memory banking
adapter that only allows mapping of the 64K over the ROM via a different
port. This is not supported.

Floppy boot requires a single density disk. The Level II ROM reads
disk 0 side 0 track 0 sector 0 (TRS80 disks are 0 offset sector count)
into 4200-42FF and then does a JP 4200	(stack is around 407D)

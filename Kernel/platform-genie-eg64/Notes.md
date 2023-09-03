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

Can we do lower case on UDG card if not on base machine ?

## Memory Map

0000-5FFF	Small parts of the kernel, video driver and data, common, constants initialized data and buffers
6000-7FFF	Always user space (used for discardable code at boot)
8000-FFFF	Banked kernel or user

We don't load 3800-3FFF or 4000-41FF. The latter is easy to fix the former
is quite tricky, so we need to load BSS space there if we use it.

## Notes

Note that there appears to have been a different 'EG MBA' memory banking
adapter that only allows mapping of the 64K over the ROM via a different
port. This is not supported.

Floppy boot requires a single density disk. The Level II ROM reads
disk 0 side 0 track 0 sector 0 (TRS80 disks are 0 offset sector count)
into 4200-42FF and then does a JP 4200	(stack is around 407D)

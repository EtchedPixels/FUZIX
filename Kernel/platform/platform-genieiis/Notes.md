# Genie IIs

TRS80 model 1 a-like but far from a straight forward clone.

## TRS80 Compatible (if mapped in)
- Keyboard (but extended)
- 64x16 video (3C00-3FFF)
- Floppy controller (model 1 style)
- Printer ?
- Option TRS80 style HD but address moved

## Not Compatible
- SIO and PIO card option for serial etc at 0xD0-D7
- Baud generator at 0xF1
- SCSI controller
- Graphics (LNW80 compatible)
- Interrupt controller

## Banked Memory

Memory banking is provided by 1-4 plug in 192K RAM cards which can be
switched into the low 48K instead of the base RAM in this area.

Port 0x7E controls the banked RAM. 
0: Set to enable banked RAM
2-3: Select card
4-5: Which block on card to map

Port 0xFE bit 7 and 2 need to set to map in external memory.

## Memory Map

### Kernel

0000-BFFF	Kernel (main memory)
(3400-3FFF	I/O temporarily when doing I/O ops)
C000-FFFF	Common

### User

0000-BDFF	User process
BE00-BFFF	Udata stash
C000-FFFF	Common Kernel

## TODO
DONE - Figure out how the card banking actually works from examples
- Double check FD code versus CP/M examples etc for density
- Boot up video init (Genie III needs it but not IIs ?)
- Loader
DONE - Save/Restore I/O mapping on IRQ
DONE - I/O mapping for console
DONE - SCSI driver
- Make trs80 hd driver a tinydisk driver of some form

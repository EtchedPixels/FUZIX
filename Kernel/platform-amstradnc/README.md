# Support for the Amstrad NC series of Z80 portable computers.

Use:

    TARGET=amstradnc/nc100 for the NC100
    TARGET=amstradnc/nc200 for the NC200

The NC100 supports hda on the PCMCIA card. The NC200 supports hda on the
PCMCIA card and fd0 on the internal floppy drive (720kB standard PC formats).
The 720kB image from fuzix.org will just work.

Use 0 to boot from PCMCIA, 256 to boot from floppy. 


## Installation to floppy

NC200 only.

Build the kernel and then dd `fuzixfloppy.img` to a raw 720kB floppy. dd a
root file system to a different floppy. Start the NC200, insert the kernel
disk, and then press YELLOW+R. The boot looader will start, load the kernel,
and you'll be left at the Fuzix bootdev prompt. Change floppies and use 256
to mount the floppy root.

**Bonus tip:** if you format your disks on the NC200 itself, it'll boot
faster (due to using non-standard sector skew).


## Installation to PCMCIA card

This currently works on the NC100 only (the NC200 won't boot from PCMCIA yet).

On a PC, do:

    $ mkfs filesystem.img 64 1408  
    $ ucp filesystem.img  
    (copy files into the filesystem here)  

then

    $ dd if=fuzix,bin of=mycard.img bs=16384  
    $ dd if=myfs of=mycard.img bs=16384 seek=20 conv=notrunc  

Now copy `mycard.img` onto the PCMCIA card. Insert into the NC100 and do
YELLOW+X to boot. At the bootdev prompt, use 0 to mount the PCMCIA file
system.


## Booting with nc100em

You can use the [nc100em](https://github.com/Nilquader/nc100em) emulator
to run the NC100 image. You don't need a set of NC100 roms.

To do this, install the emulator; then in the Fuzix distribution, do:

    $ make -C Kernel/platform-amstradnc/nc100 nc100emu.bin

Copy a card image into `~/nc100/nc100.card`, and then do:

    $ xnc100em Kernel/platform-amstradnc/nc100/nc100emu.bin

Fuzix will boot. (Note that it won't start at all unless the card image
is at least 320kB, and instead will crash obscurely.)


## NC100 memory map

16K arbitrary banks.

No CP/M emulation (due to NMI)

	0x0000	Vectors  
	0x0100  Application  
	0xEFFF  Application end  
	0xF000  Common, uarea  
	0xFFFF	Common top  (including video helpers)  

Overlaid with

	0x0000	Vectors  
	0x0100  Bootstrap code  
	0x0213	Kernel  
	0xBFFF  End of kernel space  

Overlaid at times with  
	0x4000-0x7FFF video bank (vram used by ROM OS)

NC200 is similar but CP/M should be possible

On the PCMCIA card the layout looks like

	0,1,2		Bootblocks, state, kernel  
	3,4,5		Running kernel  
	6		Initial common (boot and inherited by init)  
	8-19		User pages (12 * 16K)  
	20-63		Filesystem  


## NC200 differences

The NC200 differs from the NC100 in various ways we care about

- Video default is at 0xE000	(DONE)
- Display height is doubled	(DONE)
- Display is designed for 80 column x 16 lines in 6x8 characters (we need
  a 6x8 font and driver for this)
- 0x70 bit 2 = 0 for backlight on ?
- The keyboard is bit 3 as before but bit 4 is now power switch rather
  than it causing an NMI (so we can do CP/M emulation)
- serial interrupts are bit 2 on NC200 and bit 0 on NC100
- NC200 requires keyboard interrupts are explicitly cleared, NC100
  doesn't in fact care (but its harmless to do so) (DONE)
- Need to mask the serial interrupt when the line driver is off (may
  need that on NC100 too ideally) (DONE)
- The keymap is slightly different (see ZCN) (DONE)
- Parallel port busy is now on 0x80 bit 0 ? (DONE)
- Different rtc (same base address but a 146818 not a tc8521)
- There is a floppy drive, this is sort of described in MESS


	0x30 bit 5 set to 0 for FDC use ?  
	0x60 bit 5 is the FDC interrupt (same bit masks in 0x90 as usual)  
	0x70 bit 1 is the disk motor (1 = off)  
	0x70 bit 0 is the 765 terminal count  
	0xE0/E1 is the upd765  


## Todo

- driver for the power status bits
- lots of testing
- control doesn't seem to work ???
- pick codes for the arrow keys
- NMI/resume
- this platform would really benefit from vfork 
- floppy control/formatting
- serial format bits
- rts/cts
- dynamic buffer cache
- suspend/resume
- clean up and debug nc100 startup a bit
- full rtc support
- bigger font on nc200

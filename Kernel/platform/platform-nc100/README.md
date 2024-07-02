# Support for the Amstrad NC100/150 Z80 portable computers.

## Supported Platforms

* Amstrad NC100
* Amstrad NC150	(use the NC100 image, no serial floppy support)

## Known Bugs

The NC100 supports hda on the PCMCIA card. 

## Building

make diskimage

This will create the following image files in Images/nc100/...

* pcmcia.img: raw image (can be used with emulator) of the PCMCIA card

## Installation to PCMCIA card

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

*Note*: The Linux PCMCIA support in modern systems is generally terminally
broken and orphaned. The best approach is to load a very old Linux (2.4 era)
onto an old laptop and use that. It's also cheaper to buy an old laptop than
the various insanely priced modern adapters for PCMCIA memory cards.

## Booting with nc100 and nc200

nc100 -r nc100.rom -p pcmcia.img

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

On the PCMCIA card the layout looks like

	0,1,2		Bootblocks, state, kernel  
	3,4,5		Running kernel  
	6		Initial common (boot and inherited by init)  
	8-19		User pages (12 * 16K)  
	20-63		Filesystem  


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

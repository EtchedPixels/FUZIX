Fuzix for the ZX-Uno

To load we use the ESXDOS extension that is generally found with most
DivMMC and DivIDE interfaces. Copy FUZIX.BIN into / on the ESXDOS drive and
copy FUZIX into BIN. Then boot from ESXDOS with .fuzix

Our memory model is based on the base memory with the Timex MMU enabled. Alas
due to bugs in the ZXUNO we have to use the DIVMMC space for the low 16K which
restricts process size and generally messes things up.

Kernel
0000-3FFF	DIVMMC (low 8K RO)
4000-7FFF	Bank 5
8000-BFFF	Bank 6
C000-FFFF	Bank 3 (7 when doing video)

User

0000-3FFF	Common (Bank 4)
4000-FFFF	From dock or ex-rom (RAM) space

Graphics apps could end at BFFF and map video C000-FFFF in theory

The ZX Uno memory prioritization works like this as far as I can tell

Low 16K		DIVMMC, Dock/EXROM, +3 mode, ROM
16-32K		Dock/ExROM, +3, page 5
32-48K		Dock/ExROM, +3, page 2
48K-64K		Dock/ExROM, spectrum MMU, +3, page 0

Note: Zesarux appears to disable +3 paging when timex mode is enabled but the
Uno does not seem to.

A lot of the extra RAM is simply not accessible. We can access the following
unused space so could maybe build a swap ramdisk in future
- DIVMMC, 8K window, 112K free
- 128K memory, banks 0,1,3,4,6, 80K free
- Dock low 16K, EXROM low 16K (both rather hard to get at) 32K
and if we can use the Pentagon 256K maps then we've got another 128K but
not clear if we can. If we could it might be worth doing.

Our mapping is somewhat forced by the bugs in the ZX Uno. DIVMMC and DIVIDE
should not page in the ROM when the ZX128 ROM is the active one, but it
does. This basically forces us to run with it mapped, with the low 8K read
only after boot and the rest of the mess that goes with it. It also prevents
various other optimizations like keeping common code and the udata in the low
8K to get sensible task switching speed.

TODO:
-	debug fork/switch fail
-	optimize fork if we can
-	look at using DMA engine
-	80 column ?
-	Graphics apps loaded below C000 and with video map ?
	(need a flag to say a graphics app was loaded - missing from core)
-	Use DMA engine


Ideas on how to fix

Run 128K mode

Then we could map
0000-1FFF		Commoncode, whatever		} From DivMMC
2000-3FFF		Common data			}
4000-BFFF		Kernel code (usual 5/2)
C000-FFFF		0 for kernel, 7 for video, other options

4000-FFFF		Dock proc1
4000-FFFF		Dock proc2

Page 7			Video (512 x 192 mode)
			Font

kernel is timex mmu off
video is timex mmu off , 7
user is timex mmu on for 4000-FFFF dock or ext

Run in 64 column video, ULAplus palette

Free pages
1,3,4,6			Could use to bank kernel, external buffers etc
			= 64K
DIVMMC memory		All but 16K (112K) but same space as common
low 16K dock/ext	= 32K

(Spectranet memory is divmmc and ext/dock re-used)

total left over memory -> 208K
4 x swap slots but tricky to implement that way
Hide all the crap in a vswap device that falls back to the old swap when
full by calling down the chain to the mmc (nicely or via hack)

vswap can be mostly isolated into asm helper that knows how to find and copy
in/out page 0-n from top/bottom of ram with interrupts off

Q: top video flagged apps at 0xBFFF and map screen high (top 8 or 16K)

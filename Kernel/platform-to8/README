TO8/9+ Fuzix
=============

Memory Map:

0000-3FFF	Banked RAM 	0-31	(can be catridge ROM)
4000-5FFF	2 x 8K bank	Bank 0 in 2 8K chunks (some modes need 1 some 2 some use 2 banks)
6000-9FFF	Banked RAM
6000-60FF	Reserved for firmware (in bank mapped only ?)
6100-9FFF	Fixed RAM	1
A000-DFFF	Banked RAM	0-31	a page mapped here shows as the upper
				        8K first then lower
E000-E7AF	ROM (disc usually)
E7B0-E7FF	Devices
E800-FFFF	ROM		0-1

Pages
0	Fixed in 2x8K views at 4000-5FFF, usually video
1	Fixed at 6000-9FFF
0-3	Video must live within this range

Memory mapping:

Bank 2		:	0000-3FFF	Kernel low bank (could page but don't)
Bank 0		:	4000-5FFF	Half kernel, half video for TO9
Bank 1		:	6000-9FFF	User fixed (6000-60FF reserved by
					monitor)
Bank 4		:	A000-DFFF	Kernel upper segment
ROM/Monitor/IO	:	E000-FFFF	Including font

User Mapping
Bank 3		:	0000-3FFF	Video (except original TO9)
Bank 0		:	4000-5FFF	Common
Bank 1		:	6000-9FFF	Fixed user
Bank n		:	A000-DFFF	User banked

Limitations:
TO7/70		can't share video with kernel so not supported at this point
TO9		video is limited to single page mode as can't move video.
		Only 0-3 in low 16K (will be a different port)
TO8/9+		video must be in 0-3, 0-31 in either bank

User space banks are

6,7	first user, copy of fixed user
[5,3]	if we support 128K TO7/70 somehow later
[5,3]	also for 256K TO9 as we don't have a 16K video space but use half of 0)

8,9
A,B	to 192K

C,D
E,F	to 256K		(TO9 max)

10,11
..
1E,1F	to 512K		(TO9+)

5 currently free (except if we do 7/70 later)

Task switching requires we ldir 16K in and out of 6100-9FFF. We could do a swap
to get one extra process but it doesn't seem worth the cost except maybe on
128K - and TO7/70 is probably a unique port anyway. Could in theory also allow
bigger processes with a malloc space in the low 16K not video - but do that
later.

Requirements:
Interrupt stacks must be in common space. We rely on the low code being big
enough to push them that far up memory. All common must be above 0x4000

Drivers:

Keyboard	-	via ROM (real interface is complicated)
Light pen	-	via ROM
Mouse		-	via ROM
Video		-	vt
Floppy Disk	-	has to be via ROM. boot block is encrypted and weird.
Hard disk	-	via SDDrive and friends - slow but will have to do
			tinysd plus bitbangers
Timer int	-	via ROM hook. Push a fake RTI frame and run the full
			monitor interrupt first.


Graphics	-	TODO mode setting, ink, paper, border
			(why is "paper" not installed on fs?)
			map reporting
			ensure video is mapped on user return
			TODO: Skip drives of type 'S' as they are SDDrive?
Joystick	- 	see https://github.com/wide-dot/thomson-to8-game-engine/tree/main/engine
			or via ROM TODO (need to sort out other SD detects
			as SD on that port means no joystick present!)

TODO:
- 80 col switch via $E803 so mouse co-ords are high res ?
- debug the other sd drivers
- proper memory sizing
- video modes (80 col especially)
- floppy - how to tell SD and handle it (autoswitch or do we
  own telling it ?)

Addresses
6019	bit 0x20 informs monitor of irq status
6027 to steal int ?
6025/6 to steasl NMI
602F/30 to steal SWI1
6049	drive
604A	track
604B	track lsb
604C	sector
604E	address
6074 bit 6 = 1 periphal connected to kbd
bit 7 = 1 mouse, not light pen
60CF/D0 point to ROM font in use

ROM calls:
E004	floppy driver (or E82A for vector via monitor ?)
E806	get char in B C=1  if no char
	F8-FF info like capslock
E809	C =1 char is pressed
E818	light pen returns X 0-319 Y 0-199 cc clear if got a good measurement
E81B	light pen button CC
	set 6019 |= 2 then call with B = F8-FF for functions
E827	read joystick $00 centre $01 N $02 NE $03 E etc,
	C is set if button pressed
EC00	set palette entry 
EC08	C left button Z right mouse
EC06	X/Y for mouse X matches mode (so need to ROM switch mode ? to convince
	it to report full res ?) mouse

Mode setting is via putc
1B 48 etc

Position
1F Y X 	(Y X being |0x40)
14 hide cursor
11	show cursor
1B 4x	set colour x fg
1B 5x	ditto bg ? (fond)
1B 6x	for tour ?
1B 70-7, 78-F,80-87 do same for upper 8 colours

1B 48	page 1
1B 49	page 2
1B 4A 	superpos pg 2
1B 4B	superpos pg 1
1B 59	bitmap 4
1B 5A	40 col
1B 5B	80 col
1B 5E	16 col bitmap
1B 88-8B superposition pages 0-3

interrupts

FIR light pen
IRQ 6846 - timept, irqpt

E7E8-E7EB 6551 on RS232 card

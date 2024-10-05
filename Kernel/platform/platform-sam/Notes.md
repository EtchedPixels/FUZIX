~Sam Coupe

This port provides fairly basic support for the following configurations

	SAM Coupe with 256K or 512K of RAM (only 512K tested properly)

	Options:
		AtomLite IDE controller (8bit IDE)
		SAMBus RTC
		Printer port

	In Progress:

	Planned:
		AtomIDE controller (16bit IDE)
		Joystick
		Floppy Disk
		Serial I/O (no emulation so tricky)
		MegaRAM (as a RAM disc)

	No online docs/emulation
		Trinity
		SID


Memory layout

Kernel mode

0-7FFF		Kernel in bank 2/3
8000-FFFF	Kernel in bank 0/1

Video

0-5FFF		Frame buffer (mode 3)	bank 4/5
6000-6FFF	Font
7000-7EFF	Free
7F00-7FFF	Copy of high page in case want video high for some cases
8000-FFFF	Kernel in bank 0/1

User Space

0000-00FF	Thunking code
0100-7FFF	User page
8000-FCFF	User page
FD00-FEFF	UDATA stash
FF00-FFFF	Thunking code

Extmem support would require extra work and separate high/low pools for
the bank allocator

User Copies

0000-7FFF	User bank for copy
8000-FFFF	Kernel in bank 0/1

Forking

Need to look at the TRS80m1 hack which might actually be faster than ldir
and all the bank juggling tricks

Stub code is in the low parts of the low banks and the top of the high banks
so we have two copier routines one for each half.

	-	AtomIDE driver (prob need minimal copiers in high stubs)
	-	Serial driver
	-	Floppy driver (at least for read)

IP	-	Mode setting/graphics mode support
	-	Console colour
	-	Video ioctls
	-	UDG ioctls
	-	Is there any way to do video mapping into user process sanely
		(probably not)

	-	Sound

IP	-	MegaRAM (as RAMdisc)

	-	Maybe look at a 16K boundary aware allocator to get better
		memory packing - but it's really hairy.

	-	Less interrupt masking (but the banking logic makes it really
		foul especially as we don't have proper IM2 support)

	-	Maybe look at the modern add ons (SD card, network etc)

Boot from the created floppy .mgt file
Filesystem on Atomlite. If using simcoupe dd it 534 bytes into the image
Select device 0 for boot to get atomlite (you can do partitions)

DONE	-	Crashes on interrupt preempt (Fixed I hope)
DONE	-	Fix signal handling crashes  ( ditto )
DONE	-	Run out of pages - suggests child exit/bank32 isn't cleaning
		up right
DONE	-	Move fonts to video bank after frame buffer
DONE	-	Get to init (but not much further)
DONE	-	Debug all the new banking code and stubs
DONE	-	Atom-lite IDE driver
DONE	-	Look at how to preserve high colour bits
DONE	-	Cursor for console
DONE	-	AtomIDE lite detect fails if ROM present (select dev 0 first ?)
DONE	-	Control key debugging
DONE	-	RTC driver
DONE	-	Mouse/Joystick/Input
DONE	-	Review and test signal handling paths more (catching signals
		path appears to be broken)
DONE	-	Esc-J crashes the console driver

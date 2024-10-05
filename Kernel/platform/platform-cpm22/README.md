# Fuzix over CP/M 2.2

A FUZIX target to sit on a CP/M 2 BIOS and a customisation block

## Requirements

A system with CP/M 2.2 support, at least three memory banks that can be
mapped into the lower address space, and common space above that. The memory
banks need to be at least 32K in size. In other words a typical MP/M capable
platform.

Ideally a timer interrupt is required. If not an RTC will sort of do for
most things.

## BIOS Extensions

As the CP/M 2.2 BIOS lacks some of the needed functionality the kernel is
designed to be customised with a 1K customisation area that is headed with
a jump table in a similar style to the BIOS. The required routines are
designed to be trivial to write for any given platform.

## Limitations

The port uses the CP/M BIOS disk interface and serial interface with
additional helpers. This limits you to two serial ports (CON and AUX) and to
disk volumes that match CP/M drive letter mappings. The latter can actually
be quite convenient in a shared environment.

Memory is modelled as a set of fixed size banks with a common above. There
is no ability to make better use of things like 4 x 16K banking. Performance is
also limited by the lack of serial interrupts and the performance overhead of
the CP/M disk interface.

A system with at least three banks is required. One needs to be the full
memory space below common and enough above it to reach to the end of the
common area and BIOS. The other banks need to be at least 32K. If the memory
mapping is flexible it is possible to run on a 128K system by using the
extra memory as if it were two 32K banks.

## Advantages

For most candidate systems under 100 lines of actual assembler code are
needed, and customisation to one of systems or S100 setups where the BIOS is
the platform definition are possible.

## User Space

Fuzix Z80 binaries are relocatable and the system can therefore run existing
Fuzix Z80 binaries without any rebuilding being required. In addition,
unlike CP/M the Fuzix file system is a self describing file system
abstracted from the physical layout. It is therefore merely necessary to
build or use a file system the same size as (or smaller than) the CP/M
volume upon which it is being put.

## Versions
Two versions of the binary are built. fuzix56.com is a binary where the common
space is linked at E000-EFFF with E000-E3FF available for the customisation.
On a typical system this leaves plenty of room for the existing BIOS (usually
at F200), but on a system with fixed high RAM or similar resources it may not
be suitable. fuzix48.com places the customisation and common space at C000
instead.

Fuzix56 can be used with a common area at or below E000, fuzix48 requires a
common area at or below C000.

User 	(48K common, Exxx not Cxxx for 56K)

0000-00FF	Vectors
0100-02FF	Udata stash
0300-BFFF	User
C000-C3FF	Customisation Area
C400-		Kernel common and space

Kernel

0000-00FF	Vectors
0100-BFFF	Kernel
C000-C3FF	Customisation area
C400-		Kernel common and space


## Customisation Requirements

The binary has a 1K sysmod area that will be relocated to 0xE000 or 0xC000
according to the image chosen. It starts with a jump table of functions.

Functions are entered with a valid stack in common space and unless otherwise
documented need not save or restore the registers AF, BC, DE, HL.


The jump table entries in order are:


sysmod_init
		called during initial set up. This can do whatever is needed
		to set up the customisation and turn on interrupt timers.

sysmod_info	
		return the sysinfo structure (see below) in HL

sysmod_set_map
		Set the banking map to the map in A. Maps are (to the OS)
		numbered sequentially from 0, where 0 is the CP/M TPA. Must
		save and restore any register changed except AF. Must behave
		correctly if re-entered during an interrupt.

sysmod_irq_handler
		Install the OS IRQ handler HL. This should be called whenever
		there is a timer event. Extending this to other events will
		happen later. If the system has its own IRQ handler then
		clean the stack back up and end the existing handler with
		a JP to the address given rather than RET or RETI. The
		handler passed will be in common space.

sysmod_nmi_handler
		Install the OS NMI handler. Can be a no-op if the NMI handler
		is needed by the system. The OS handler merely displays a
		diagnostic.

sysmod_conost
sysmod_auxist
sysmod_auxost
		CP/M 3 equivalent for the missing 2.2 function

sysmod_idle
		Called whenever the system is idling. Can be a no-op or
		on interrupt driven systems and interrupt etc. The OS will
		also repeatedly poll the consoles while idle to maximise
		responsiveness.

sysmod_rtc_secs
		On a platform with an RTC return the seconds field (0-59) in
		L. If no RTC is present return 0xFF in L. This is used both
		to handle timing when no timer is present and to lock the
		system to the actual time and avoid drift. If the timer
		is temporarily unavailable return 0xFF.

sysmod_monitor
		Exit the OS into a monitor if available, if not reboot

sysmod_reboot
		Reboot the system, if not possible just stop (eg di halt)

sysmod_conconf
		Configure the console port according to the properties
		passed in HL. HL should be returned giving the actual
		configuration set. For example if the user asks for 50 baud
		but cannot be given it then HL should encode the baud
		rate actually set.

sysmod_auxconf
		Configure the auxiilary port according to the properties
		passed in HL. HL should be returned giving the actual
		configuration set.

sysmod_joystick
		Returns the joystick bits in HL where H is JS0 and L is JS1.
		Return 0 for absent sticks.

		The bits for button/direction are
		7:	up
		6:	down
		5:	left
		4:	right
		0-3:	button 0-3

		Where a 1 bit means pressed/active

Serial properties
	bits 3-0	:	speed
		0	Not in use
		1	50
		2	75
		3	110
		4	134
		5	150
		6	300
		7	600
		8	1200
		9	2400
		A	4800
		B	9600
		C	19200
		D	38400
		E	57600
		F	115200
	bit 5-4
		0	5 bit
		1	6 bit
		2	7 bit
		3	8 bit
	bit 6
		1	two stop bits
		0	one stop bit
	bit 7
		0	receiver disabled
		1	receiver enabled
	bit 8
		0	no parity
		1	parity enabled
	bit 9
		0	even parity (if parity)
		1	odd parity (if parity)
	bit 10-11
		Internal OS use
	bit 12
		0	no flow control
		1	hardware flow control
	bits 13-15
		reserved



SysInfo structure
		byte	banks		; number of banks present
		byte 	features	; feature bits (see below)
		byte	tickrate	; number of irq calls per 1/10th sec
		byte	swap		; drive for swap (0xFF if not)
		word	commonbase	; first address in the common space
		word	conflags	; console changable serial properties
		word	auxflags	; aux changeable serial properties
					; set bits are supported changes

		Drives are encoded 0-15 for A-P.

Feature flags allocated
		bit 0:	set if auxist/auxost present and aux port should be
			a second tty device
		bit 1:  a timer is present
		bit 2:	disable interrupts during CP/M READ and WRITE calls
		bit 3:	disable interrupts during CP/M CONOUT
		bit 4:	direct floppy disk I/O via a bounce buffer
		bit 5:	direct hard disk I/O via a bounce buffer

Patching In The SysMod

The .COM file for the OS load starts with

````
	JR	xx
	DEFW	0x1DEA
	DEFW	offset
````

Where offset is the offset into the file of the customisation area

## Interrupt Handling

The only provided interrupt hook at this point is for the timer (serial will
be addressed in future). The handler expects to be called as the sole
interrupt handler or at the tail of any existing handlers. It may switch
tasks and perform other activity. If there is no existing handler required
then setting 0x38 to a JP and 0x39/40 to the given vector is normally
sufficient. If a handler exists then terminating the existing handler with
a JP to the OS handler instead of a RET will do the right thing. If the
existing handler cannot be patched then it usually works to generate a new
interrupt handling stub of the form

````
                   CALL old_handler
                   JP XXXX
````

where XXXX is then patched with the address provided by Fuzix. In systems
using IM2 this may not work.

Interrupts may occur during a CP/M or customisation function. This is not
normally a problem however there are two cases to be careful of.

1. Some platforms have floppy disk routines that assume there is no
interrupt and do not block interrupts during critical read and write code.

2. If you have a platform where the underlying firmware also stack switches
to private memory mappings (for example to map video memory). This is
currently hard to handle but may change in future.

For the two common cases (console output and floppy) there are feature bits
that will disable interrupts during those calls.

## Re-entrancy

The CP/M BIOS is never re-entered. The customisation functions are not
re-entered except for sysmod_set_map which must always do the right thing.

## Replacing CP/M BIOS Functions

No provision is made for this as the sysmod_init function is perfectly able
to patch the BIOS jump tables if desired.

## DMA

Most CP/M systems do not use DMA. On a system which uses DMA the use of the
CP/M BIOS while bank switched may not place the data in the right location.
On such systems for the affected devices it is necessary to wrap the BIOS
READ and WRITE methods to use a bounce buffer in high memory. Feature bits
can be set to bounce floppy or hard disk I/O through a common buffer - but
as always at a speed cost. In some cases it may be possible to load the
DMA bank registers in sysmod_set_map instead.

## Serial Interrupts

Currently there is no real provision for serial interrupt handling. If the
BIOS has buffered interrupt driven serial this will however work.

## Possible Future Directions

- Support for direct serial interrupt handling
- Optional video and keyboard interface (as opposed to virtual serial)
- Optional direct disk interface for high speed devices
- Support for single user bank systems

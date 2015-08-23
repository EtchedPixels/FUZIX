# SDD banking

SDCC itself does not support code banking, only data banking. Even the data
banking isn't truely "far" pointers so is mostly useless.

To do code banking Fuzix uses a modifed version of the SDCC 3.4.1 linker
code. This avoids the need to specify far functions and banks inline in the
code itself an allows us to fix up the banking semi-automatically. Currently
it's not smart enough to lay out the banks itself.

The compiler is also modified to add an option --external-banker which
causes it to do two things

1.  Calls to functions are generated with push af/pop af either side
	of the invocation (except calls to literals)

2. Functions assume there are four bytes of pushed address


## How it works

sdldz80 takes a -r flag. In the presence of the -r flag it assumes for now

Code Segments:

	_CODE		is "bank 1"
	_CODE2		is "bank 2"
	_CODE3		is "bank 3"
	_VIDEO		is "bank 3"
	_DISCARD	is common (bank 0)
	_COMMONMEM	is common (bank 0)

Data Segments:

	_CONST		is common (bank 0)
	_INITIALIZED	is common (bank 0)
	_DATA		is common (bank 0)
	_FONT		is "bank 3"
	_COMMONDATA	is common (bank 0)

To be added is the notion of data segments containing only pointer
references in segment. In particular so we can add _CONST2 which is data
in bank2 for functions in bank2, and referenced only from bank2.

That will improve syscall table behaviour.

For each relocation sdldz80 checks if it is a relocation between two banks
and the target bank is non zero. If it is then it outputs an entry
giving the relocation information.

For all relocations the linker then performs a normal unbanked fixup. The
linker outputs all the data into a bihx format file which is basically
packaged ihx. bihx then splits it into a set of ihx files for each bank plus
relocs.dat which is the relocations. The ihx files are turned into binary
images.

binmunge reads the relocs and banks and converts as follows

Code

	PUSH AF
	CALL xx
	POP AF

is turned into

	CALL __bank_n_from_m	; for inter-bank calls
	DEFW xx

	0xC3 (JUMP) is turned into a jump into a stub

Anything else is considered an error

Data
	Each code entry is turned into a stub. Identical entries are turned
	into the same stub call from the same bank (They are assumed to be
	function pointers)

Only 16bit relocations are processed. Weird tricks like &function >> 8 may
break. Hopefully SDCC never decides to create one behind our back.

Stubs:

either

	LD DE, #function
	JP __stub_n_from_m

or

	LD HL, #function
	JP __stub_n_from_0


The stubs may live in the bank of the invoking function, while __bank_x is
common.

This is done to process tables. It means that function tables like device or
syscall tables correctly generate inter bank calls.

Banking Handlers

See [platform-zx128/zx128.s](https://github.com/EtchedPixels/FUZIX/blob/master/Kernel/platform-zx128/zx128.s)

Assumptions

	__sdcc_callhl
	__enter
	__enter_s

must live in common memory

You must use a sdcc support library modified to expect 'far' style offsets
on helpers, but they may be banked.

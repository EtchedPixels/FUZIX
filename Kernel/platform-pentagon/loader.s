;
;	Load our kernel image via TRDOS
;
;	The disk holds files of the format code{n} where n is the bank to
;	place it in. TRDOS is asked to load the lot and then we launch
;

		.module loader
		.area CODE(ABS)
		.org 0x7800

start:
		ld sp,#start
		xor a
		ld de,#0x0109
		call trdos		; bank 0	(will appear at 0x0000)
		inc a
		ld de,#0x0509
		call trdos		; bank 1	(high bank CODE1)
		inc a
		ld de,#0x0909
		call trdos		; bank 2	(0x8000-0xBFFF bank)
		ld a,#6
		ld de,#0x0D09
		call trdos		; bank 6	(high bank CODE2)
		inc a
		ld de,#0x1109
		call trdos		; bank 7	(high bank CODE3)

		di
		; Turn off low ROM
		; Some older systems want 0x1FFD bit 0 instead FIXME
		ld bc,#0xeff7
		ld a,#0x08
		out (c),a
		; The kernel is loaded into 1,6,7
		; The low memory is loaded into bank 0
		; The 8000-BFFF range is loaded by the loader
		; The 4000-7FFF range is zero

		; Switch ROM for bank 0
		ld a,#0x01
		ld bc,#0x7ffd
		out (c),a
		; FIXME - where is best to start up
		jp 0xC000

trdos:
		; Switch bank
		push af
		ld bc,#0x7ffd
		out (254),a
		or #0x10
		out (c),a
		ld hl,#0xC000
		ld bc,#0x4005
		call 15635
		pop af
		ret

;
;	Most of the work was done by the BASIC loader. At this point in
;	time banks 0 6 and 7 are correct, whilst banks 3 and 4 hold the
;	other stuff we need
;

		.module loader
		.area CODE(ABS)
		.org 0x6000

start:
		di
		; Turn off low ROM
		; Some older systems want 0x1FFD bit 0 instead FIXME
		ld bc,#0xeff7
		ld a,#0x08
		out (c),a
		; The kernel is loaded into 1,6,7
		; The low memory is loaded into bank 3
		; The 8000-BFFF range is loaded by the loader
		; The 4000-7FFF range is zero
		ld a,#0x03
		ld bc,#0x7ffd
		out (c),a
		ld hl,#0xc000
		ld de,#0x0000
		ld bc,#0x4000
		ldir
		ld a,#0x01
		ld bc,#0x7ffd
		out (c),a
		; FIXME - where is best to start up
		jp 0xC000

	.area LOADER(ABS)

	.org 0xEE00

start:
	ld hl, #0x0100
	ld de, #0xEE00
	ld bc, #0x0200
	ldir
	jp go			; Into the high space
go:
	ld a,#0x80
	out (0),a
	ld sp, #0xEE00

	ld hl,#0x0100
	ld (0xffee),hl
	ld b,#4			; Bin the rest of the boot sector
	call 0xfff9
	ld hl,#0x0100
	ld (0xffee),hl
	ld b,#0xFE		; Load 0100->7FFF
	call 0xfff9
	ld hl,#0x8000
	ld (0xffee),hl
	ld b,#0x70		; Load 8000->B7FF
	call 0xfff9
	; We have to miss a bit for now because the CF ROMs borrow
	; B8-BF
	ld hl,#0xC000
	ld (0xffee),hl
	ld b,#0x50		; Load C000->E7FF (really B800-DFFF)
	call 0xfff9
	;
	; Undo the hole now we no longer need the BIOS helpers
	;
	ld hl,#0xC000
	ld de,#0xB800
	ld bc,#0x2800
	ldir
	jp 0x0100

;	Bootstrap loader (start of first ROM segment used). Our job
;	is to get the rest of the OS image that is RAM based loaded into
;	RAM

	.module bootstrap

	.area	    BOOT	(ABS)

	.org	    0xC000
	jp	start
	.ascii  "#"
	.byte	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
	.byte	0
	jp	start
	.ascii	"FUZIX"
	.byte	0x20, 0x20, 0x20
	.byte	0, 0

.macro BANKCHK
	ld	hl,#0xE7FF
	cp	(hl)
	jp	nz, 0x3E00
	dec	hl
	cpl
	cp	(hl)
	jp	nz, 0x3E00
	cpl
.endm
;
;	The ROM should be pageable, the RAM may already be paged in or maybe
;	not.
;
start:
	ld	hl,#bankbad
	ld	de,#0x3E00
	ld	bc,#0x0200
	ldir
	jp	0x3E02

;
;	This code needs to be relocatable, or we need to change how we link
;	and build this lot
;
bankbad:				; Will be at 3F00
	di
	halt
bootstrap:
	di

	ld	a,#5
	out	(0xFF),a		; Switch ROM to bank 5
	BANKCHK
	ld	hl,#0xC000
	ld	de,#0x1000
	ld	bc,#0x2000
	ldir
	ld	hl,#0x1000
	ld	de,#0x4000
	ld	bc,#0x2000
	ld	(0xFC00),a		; ROM off
	out	(7),a			; RAM in
	out	(4),a			; Force foreground block
	ldir				; Install first half of _CODE
	out	(6),a			; Back to ROM
	ld	(0xF800),a

	ld	a,#6
	out	(0xFF),a		; Switch ROM to bank 6
	BANKCHK
	ld	hl,#0xC000		; Copy second block
	ld	de,#0x1000
	ld	bc,#0x2000
	ldir
	ld	hl,#0x1000
	ld	de,#0x6000
	ld	bc,#0x2000
	ld	(0xFC00),a		; ROM off
	out	(7),a			; RAM in
	ldir				; Install second half of _CODE

	out	(6),a			; Back to ROM
	ld	(0xF800),a
	ld	a,#7
	out	(0xFF),a		; Switch ROM to bank 7
	BANKCHK
	ld	hl,#0xC000		; Copy discard
	ld	de,#0x1000
	ld	bc,#0x2800
	ldir
	ld	hl,#0x1000
	ld	de,#0x8200
	ld	bc,#0x2800
	ld	(0xFC00),a		; ROM off
	out	(7),a			; RAM in
	ldir				; Install _DISCARD
	out	(6),a			; Back to ROM
	ld	(0xF800),a

	xor	a
	out	(0xFF),a		; Switch ROM to bank 0
	BANKCHK
	ld	hl,#0xC200		; Copy common etc
	ld	de,#0x0400
	ld	bc,#0x2600		; 9.5K should be sufficient
	ldir

	ld	a,#1
	out	(0xFF),a		; Switch ROM to bank 1
	BANKCHK
	ld	a,#2
	out	(0xFF),a		; Switch ROM to bank 2
	BANKCHK
	ld	a,#3
	out	(0xFF),a		; Switch ROM to bank 3
	BANKCHK
	ld	a,#4
	out	(0xFF),a		; Switch ROM to bank 4
	BANKCHK

	; We now have the program in the right places
	; ROM on. RAM in.

	out	(7),a			; RAM in
	jp	0x4000

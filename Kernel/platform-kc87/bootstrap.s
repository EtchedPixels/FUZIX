;
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
;
;	The ROM should be pageable, the RAM may already be paged in or maybe
;	not.
;
start:
	ld	hl,#bootstrap
	ld	de,#0x3F00
	ld	bc,#0x0100
	ldir
	jp	0x3F00

;
;	This code needs to be relocatable, or we need to change how we link
;	and build this lot
;
bootstrap:
	ld	a,#5
	out	(0xFF),a		; Switch ROM to bank 5
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
	ld	hl,#0xC200		; Copy common etc
	ld	de,#0x0400
	ld	bc,#0x2600		; 9.5K should be sufficient
	ldir

	; We now have the program in the right places
	; ROM on. RAM in.
	out	(7),a			; RAM in
	jp	0x4000

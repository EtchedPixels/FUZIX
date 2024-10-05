.z80
;
;	Genie II bootblock (256 bytes)
;
;	Load tracks 1+ from 0100-FDFF, deal with the fact that we have
;	MMIO by buffering the block and copying each sector with the
;	map changed after the read.
;
;
.area	    BOOT	(ABS)
.org	    0xFF00		; We run as a "diagnostics" boot
;
;	The 2791 is memory mapped
;
FDCREG	.equ	0x37EC
FDCTRK	.equ	0x37ED
FDCSEC	.equ	0x37EE
FDCDATA	.equ	0x37EF
;
;	Drive select is also more complicated than other models
;
LATCHD0	.equ	0x37E1		; also drive select
LATCHD1	.equ	0x37E3
LATCHD2	.equ	0x37E5
LATCHD3	.equ	0x37E7

start:
	xor	a	
	out	(0xFE),a	; ROM and MMIO left in

	ld	sp,#0xFE00
	ld	hl,#0x3C00
	ld	(hl),#"G"
	ld	de,#0x0100	; Binary address in other bank

	exx
	ld de,	#FDCDATA	; data port
	ld hl,	#FDCREG		; command port

;
;	Seek to track
;
lastsec:
	ld	a, #0			; self modifying to save space
	inc	a
	ld	(lastsec+1), a		; track number... (start on 1)
	ld	(de),a			; Desired track into data
	ld	(hl),#0x1B		; seek, lowest step speed
	push	bc
	ld	b, #0			; 0x00 256 delays
cmd1:
	djnz	cmd1
seekstat:
	pop	bc
	bit	0, (hl)
	jr	nz, seekstat
	ld	a,(hl)
	and	#0x18
	jr	z, secmove
	exx
	ld	a,#'S'
bad:
	ld	(0x3C00),a
badl:
	jr	badl
;
;	To our sector (already on track)
;
secmove:
	xor	a
	dec	a	
	ld	(nextsec+1), a
nextsec:
	ld	a, #255		; self modifying sector count
	inc	a
	ld	(nextsec+1), a
	cp	#10
	jr	z, lastsec
	call	floppy_read
	;	Data loaded to 0xFE00-FEFF
	;	Unmap the MMIO and put the block in place
	ld	a,#0x05
	out	(0xFE),a
	exx
	ld	hl,#0xFE00	; Disk buffer
	ld	bc,#0x0100
	ldir			; Copy block to DE and adjust DE
	ld	a,#0xFE
	cp	d
	jp	z, 0x0100
	xor	a
	out	(0xFE),a
	exx
	jr	nextsec
;
;	Assume the boot ROM left us on track 0 and motor on
;
floppy_read:
	ld	(FDCSEC), a		; sector please
	ld	a, #0x01		; drive 0 select
	ld	(LATCHD0), a		; keep motor on
	ld	a, #0x8C		; READ
	ld	(hl), a
	ld	b, a			; 8C is an acceptable delay!
l1:	djnz	l1
	ld	bc,#0xFE00		; Sector buffer
flopin:	
	bit	1,(hl)
	jr	z, flopin
	ld	a,(de)
	ld	(bc),a
	inc	c			; page aligned
	jr	nz,flopin
flopstat:
	ld	a, (hl)
	and	#0x19
	rra				; test bit 0
	jr	nz, flopstat
	or	a			; safe even though we rotated right
	ret	z
	ld	a,#'!'
	jr	badl

	.org	0xFFE0

	.db	0x03			; Load us at FF00 please

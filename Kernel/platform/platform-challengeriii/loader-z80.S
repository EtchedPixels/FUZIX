;
;	Floppy boot
;
;	Must be relocatable. Called after the 6502 changeover
;
	.abs
	.z80

	.org	0x2400

begin_z80:
	ld	sp,0xDFFF
	ld	hl,0x2440	; main loader is packed here
	ld	de,0xD000
	ld	bc,512
	ldir
	jp	0xD000

; Our packer will put all these bits together

	.org	0xD000

	; This bit must be relocatable unless you want to fiddle with
	; the stuff above
	; load 2K off each track (so it works 5.25" or 8")
run_z80:
	ld	hl,0xF701
	ld	(hl),4		; Select data
	ld	a,0x0F
	ld	(0xF700),a	; Data to 0F
	ld	(hl),0		; Select DDR
	ld	a,0x0F
	ld	(0xF700),a	; Low 4 writable
	ld	(hl),4		; Set back to data

	; Begin loading into the RAM now it is paged properly
	ld	hl,0x0000
	ld	bc,0x1801	; load 48K, start at track 1
dnext:
	call	dload
	inc	c
	djnz	dnext
	ld	a,13
	call	outch
	ld	a,10
	call	outch
	ld	a,13
	call	outch
	ld	a,10
	call	outch
	; Code should be present and good
	jp	0x0100

;
; Now read the track like a tape
; Uses the alt regs, moves HL on. preserves BC
;
dload:
	exx
	; Step in, read, rinse repeat
	ld	hl, 0xC002
	ld	(hl),0xFB
	ld	(hl),0xF3
	ld	(hl),0xFB
	ld	(hl),0x7F	; load head
	call	snore		; step and settle delay
retry:
	ld	hl, 0xC000
widx:
	ld	(0xC020),a	; ping motor
	bit	7,(hl)
	jr	nz, widx
widxe:
	ld	(0xC020),a
	ld	a,(0xC011)
	bit	7,(hl)
	jr	z, widxe
	ld	a,0x03
	ld	(0xC010),a
	ld	a,0x58
	ld	(0xC010),a
whdr:
	call	dbyte
	cp	0x57
	jr	nz, whdr
	call	dbyte	; track id (TODO check)
	ld	bc,0x0008
	ld	de,0
rdat:
	call	dbyte
	exx
	ld	(hl),a
	inc	hl
	exx
	add	a,e
	ld	e,a
	jr	nc,noripple
	inc	d
noripple:
	djnz 	rdat
	dec	c
	jr	nz, rdat
	call	dbyte
	cp	e
	jr	nz, badsum
	call	dbyte
	cp	d
	jr	nz, badsum
	ld	a,'.'
	call	outch
	exx
	ret
badsum:
	ld	a,'E'
	call	outch
	jr	retry

outch:
	ld	e,a
outchw:
	ld	a,(0xFC00)
	and	2
	jr	z, outchw
	ld	a,e
	ld	(0xFC01),a
	ret

dbyte:
	ld	a,(0xC010)
	rra
	jr	nc,dbyte
	ld	a,(0xC011)
	ret

snore:	; wait about 10ms @4MHz
	; TODO
	push	bc
	ld	bc,616
	; 65 cycles a loop + start/end overhead
loop:	ex	(sp),hl		;	19
	ex	(sp),hl		;	19
	dec	bc		;	7
	ld	a,b		;	4
	or	c		;	4
	jr	nz, loop	;	12/7
	pop	bc
	ret

;
;	LOBO Max Boot Block
;
;	We are loaded from disk at 0x0400 and the ROM provides us helpers
;	to load more. Has to fit in 256 bytes
;
;	On entry interrupts are off
;
	.abs
	.org	0x0400

stack	.equ	0x700		; somewhere safe above us in the ram window
start:
	jr	go

	.ds	16		; Review ROM TODO
go:
	ld	sp, stack

	;	Video RAM please
	ld	a,2
	call	wipe
	;	Font
	ld	a,4
	call	wipe
	ld	a,0x7E
	ld	hl,0x0008
	ld	b,8
block:
	ld	(hl),a
	inc	hl
	djnz	block

	; Upper video RAM
	ld	hl,0x0C00
	call	wipe1k

	;	Set the video controller up first
	ld	de, crtdat
	ld	bc, 0x1000
	ld	hl, 0x07E0
setcrt:
	ld	(hl),c
	inc	hl
	ld	a,(de)
	inc	de
	ld	(hl),a
	dec	hl
	inc	c
	djnz	setcrt

	ld	ix, 0x0C00

	xor	a
	ld	(0x07DC),a	; ROM back

	;
	;	Do the load
	;
	ld	hl, 0x1000	; To 0x1000
	ld	de, 0		; Track 0 Head 0
	ld	bc, 0x0101	; Sector 1-1 (sector 0 of first track was
				; this boot code)

next:
	push	hl
	push	de
	push	bc

	call	0003		; ROM helper

	ld	(ix + 0),1
	inc	ix

	pop	bc
	pop	de
	pop	hl

	inc	h		; 512 byte sectors
	inc	h

	jr	z, done		; Loaded from 0x1000-0xFFFF

	inc	b
	inc	c
	ld	a,10
	cp	b
	jr	nz, next

	;	We boot single sided

	;	Next track
	inc	de
	ld	bc,0		; back to sector 0
	jr	next

done:
	jp	0x1000		; into loaded kernel

wipe:	;	Wipe the 1K swap1k buf
	ld	hl,0x0000
	ld	(0x07DC),a
wipe1k:	;	or anywhere
	ld	d,h
	ld	e,l
	ld	(hl),0
	inc	de
	ld	bc,0x3FF
	ldir
	ret

crtdat:
	.byte	119, 80, 97, 6, 27, 12, 24, 26, 0, 8, 0, 0, 0, 0, 0, 0

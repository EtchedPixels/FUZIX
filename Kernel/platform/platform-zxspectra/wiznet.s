
; Do we need interrupts off ?

w5x00_readcb:
	pop	de
	pop	hl
	push	hl
	push	de
	ld	bc,#0x013B
	ld	a,#WIZNET
	out	(c),a
	ld	hl,#0x2000
	add	hl,de
	ld	l, (hl)
	ld	a,#0xC5
	out	(c),a
	ret
	
w5x00_readcw:
	pop	de
	pop	hl
	push	hl
	push	de
	ld	bc,#0x013B
	ld	a,#WIZNET
	out	(c),a
	ld	hl,#0x2000
	add	hl,de
	ld	a, (hl)
	inc	hl
	ld	l,(hl)
	ld	h,a
	ld	a,#0xC5
	out	(c),a
	ret

;	bread(bank, offset, ptr, count)
w5x00_bread
;	Ditto with user map
w5x00_breadu

;	Off, value
w5x00_writecb
w5x00_writecw
;	bank, offset, ptr, count
w5x00_bwrite
w5x00_bwriteu
w5x00_setup

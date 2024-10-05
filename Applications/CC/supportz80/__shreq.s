;
;	TOS is the lval, hl is the shift amount
;
;
		.export __shreq
		.code

__shreq:
		ex	de,hl		; shift into de
		pop	hl
		ex	(sp),hl		; pointer into hl

		ld	a,e	; save shift value
		ex	de,hl
		ld	e,(hl)
		inc	hl
		ld	d,(hl)

		; No work to do
		and	15
		ret	z

		push	bc
		ld	b,a

shuffle:
		sra	d
		rr	e
		djnz	shuffle
shdone:
		pop	bc
		ld	(hl),d
		dec	hl
		ld	(hl),e
		ex	de,hl
		ret

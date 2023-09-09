;
;	Subtract de from (HL), return the result
;
		.export __minuseqde
		.code

__minuseqde:
		push	bc
		ld	b,d	; save subtraction value into BC
		ld	c,e
		ld	a,(hl)
		inc	hl
		ld	d,(hl)
		sub	c
		ld	e,a
		ld	a,d
		sbc	a,b
		ld	d,a
		ld	(hl),d
		dec	hl
		ld	(hl),e
		ex	de,hl
		pop	bc	; restore BC
		ret

;
;		(HL) &= DE
;
		.export __xoreqde
		.code

__xoreqde:
		ld	a,(hl)
		xor	e
		ld	(hl),a
		ld	e,a
		inc	hl
		ld	a,(hl)
		xor	d
		ld	(hl),a
		ld	d,a
		ex	de,hl
		ret


;
;		(HL) &= DE
;
		.export __xoreqde
		.export __xoreqde0d
		.code

__xoreqde0d:
		ld	d,0
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


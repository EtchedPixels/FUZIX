;
;		(HL) &= DE
;
		.export __oreqde
		.export __oreqde0d
		.code

__oreqde0d:
		ld	d,0
__oreqde:
		ld	a,(hl)
		or	e
		ld	(hl),a
		ld	e,a
		inc	hl
		ld	a,(hl)
		or	d
		ld	(hl),a
		ld	d,a
		ex	de,hl
		ret


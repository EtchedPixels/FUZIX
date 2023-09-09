;
;		(HL) &= DE
;
		.export __oreqde
		.code

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


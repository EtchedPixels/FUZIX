;
;		(HL) &= DE
;
		.export __andeqde
		.export __andeqde0d

		.code

__andeqde0d:
		ld	d,0
__andeqde:
		ld	a,(hl)
		and	e
		ld	(hl),a
		ld	e,a
		inc	hl
		ld	a,(hl)
		and	d
		ld	(hl),a
		ld	d,a
		ex	de,hl
		ret


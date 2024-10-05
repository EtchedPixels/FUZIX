;
;	HL = lval
;
		.export __pluseq1
		.export __pluseq2
		.export __pluseq1d
		.export __pluseq2d
		.code

__pluseq1d:
		ex	de,hl
 __pluseq1:
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		inc	de
		ld	(hl),d
		dec	hl
		ld	(hl),e
		ex	de,hl
		ret

__pluseq2d:
		ex	de,hl
__pluseq2:
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		inc	de
		inc	de
		ld	(hl),d
		dec	hl
		ld	(hl),e
		ex	de,hl
		ret

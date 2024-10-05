		.export __postinc1
		.export __postinc2
		.export __postinc1d
		.export __postinc2d
		.code

__postinc1d:
		ex	de,hl
__postinc1:
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		inc	de
		ld	(hl),d
		dec	hl
		ld	(hl),e
		ex	de,hl
		dec	hl
		ret

__postinc2d:
		ex	de,hl
__postinc2:
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		inc	de
		inc	de
		ld	(hl),d
		dec	hl
		ld	(hl),e
		ex	de,hl
		dec	hl
		dec	hl
		ret

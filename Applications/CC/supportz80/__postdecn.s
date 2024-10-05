		.export __postdec1
		.export __postdec2
		.export __postdec1d
		.export __postdec2d
		.code

__postdec1d:
		ex	de,hl
__postdec1:
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		dec	de
		ld	(hl),d
		dec	hl
		ld	(hl),e
		ex	de,hl
		inc	hl
		ret

__postdec2d:
		ex	de,hl
__postdec2:
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		dec	de
		dec	de
		ld	(hl),d
		dec	hl
		ld	(hl),e
		ex	de,hl
		inc	hl
		inc	hl
		ret

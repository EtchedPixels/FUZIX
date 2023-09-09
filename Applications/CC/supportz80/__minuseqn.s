;
;	HL = lval
;
		.export __minuseq1
		.export __minuseq2
		.export __minuseq1d
		.export __minuseq2d

		.code

__minuseq1d:
		ex	de,hl
__minuseq1:
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		dec	de
		ld	(hl),d
		dec	hl
		ld	(hl),e
		ex	de,hl
		ret

__minuseq2d:
		ex	de,hl
__minuseq2:
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		dec	de
		dec	de
		ld	(hl),d
		dec	hl
		ld	(hl),e
		ex	de,hl
		ret

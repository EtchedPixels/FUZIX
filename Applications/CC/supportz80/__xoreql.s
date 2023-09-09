		.export __xoreql
		.code
__xoreql:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		ld	a,e
		xor	(hl)
		ld	e,a
		ld	(hl),a
		inc	hl
		ld	a,d
		xor	(hl)
		ld	d,a
		ld	(hl),a
		inc	hl
		push	de		; save the lower result
		; Upper word
		ld	de,(__hireg)
		ld	a,e
		xor	(hl)
		ld	e,a
		ld	(hl),a
		inc	hl
		ld	a,d
		xor	(hl)
		ld	(hl),a
		ld	h,a
		ld	l,e
		ld	(__hireg),hl
		pop	hl
		ret

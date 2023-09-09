		.export __andeql
		.code
__andeql:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		ld	a,e
		and	(hl)
		ld	e,a
		ld	(hl),a
		inc	hl
		ld	a,d
		and	(hl)
		ld	d,a
		ld	(hl),a
		inc	hl
		push	de		; save the lower result
		; Upper word
		ld	de,(__hireg)
		ld	a,e
		and	(hl)
		ld	e,a
		ld	(hl),a
		inc	hl
		ld	a,d
		and	(hl)
		ld	(hl),a
		ld	h,a
		ld	l,e
		ld	(__hireg),hl
		pop	hl
		ret


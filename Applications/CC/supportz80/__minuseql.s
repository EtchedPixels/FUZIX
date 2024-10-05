		.export __minuseql
		.code

__minuseql:
		ex	de,hl
		pop	hl
		ex	(sp),hl

		; HL is pointer, hireg:de amount to subtract

		ld	a,(hl)
		sub	e
		ld	(hl),a
		ld	e,a
		inc	hl
		ld	a,(hl)
		sbc	a,d
		ld	(hl),a
		ld	d,a
		inc	hl
		push	de

		ld	de,(__hireg)

		ld	a,(hl)
		sbc	a,e
		ld	(hl),a
		ld	e,a
		inc	hl
		ld	a,(hl)
		sbc	a,d
		ld	(hl),a
		ld	d,a

		ld	(__hireg),de

		pop	hl
		ret

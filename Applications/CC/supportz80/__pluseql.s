		.export __pluseql
		.code

__pluseql:
		ex	de,hl
		pop	hl
		ex	(sp),hl

		; HL is pointer, hireg:de amount to add

		ld	a,(hl)
		add	a,e
		ld	(hl),a
		ld	e,a
		inc	hl
		ld	a,(hl)
		adc	a,d
		ld	(hl),a
		ld	d,a
		inc	hl
		push	de

		ld	de,(__hireg)

		ld	a,(hl)
		adc	a,e
		ld	(hl),a
		ld	e,a
		inc	hl
		ld	a,(hl)
		adc	a,d
		ld	(hl),a
		ld	d,a

		ld	(__hireg),de

		pop	hl
		ret

;
;		32bit complement
;
		.export __cpll
		.code

__cpll:
		ld	a,h
		cpl
		ld	h,a
		ld	a,l
		cpl
		ld	l,a
		push	hl
		ld	hl,(__hireg)
		ld	a,h
		cpl
		ld	h,a
		ld	a,l
		cpl
		ld	l,a
		ld	(__hireg),hl
		pop	hl
		ret

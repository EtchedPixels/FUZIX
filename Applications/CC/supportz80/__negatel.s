;
;		32bit negate
;
		.export __negatel
		.code
__negatel:
		ld	a,h
		cpl
		ld	h,a
		ld	a,l
		cpl
		ld	l,a
		inc	hl
		ld	a,h
		or	a
		push	hl
		push	af
		ld	hl,(__hireg)
		ld	a,h
		cpl
		ld	h,a
		ld	a,l
		cpl
		ld	l,a
		pop	af
		jr	nz,nocarry
		inc	hl
nocarry:
		ld	(__hireg),hl
		pop	hl
		ret

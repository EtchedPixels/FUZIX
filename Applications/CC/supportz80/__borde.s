		.export __borde
		.code

__borde:
		ld	a,d
		or	h
		ld	h,a
		ld	a,e
		or	l
		ld	l,a
		ret

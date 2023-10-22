		.export __borde
		.export __borde0d
		.code

__borde:
		ld	a,d
		or	h
		ld	h,a
__borde0d:
		ld	a,e
		or	l
		ld	l,a
		ret

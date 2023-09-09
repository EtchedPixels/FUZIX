		.export __bxorde
		.code

__bxorde:
		ld	a,d
		xor	h
		ld	h,a
		ld	a,e
		xor	l
		ld	l,a
		ret

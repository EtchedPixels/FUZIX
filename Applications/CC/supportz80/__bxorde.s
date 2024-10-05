		.export __bxorde
		.export __bxorde0d
		.code

__bxorde:
		ld	a,d
		xor	h
		ld	h,a
__bxorde0d:
		ld	a,e
		xor	l
		ld	l,a
		ret

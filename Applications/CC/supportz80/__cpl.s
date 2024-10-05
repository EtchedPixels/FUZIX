		.export	__negate
		.export __cpl
		.code

__negate:
		dec	hl
__cpl:
		ld	a,h
		cpl
		ld	h,a
		ld	a,l
		cpl
		ld	l,a
		ret

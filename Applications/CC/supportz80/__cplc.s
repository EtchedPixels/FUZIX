		.export	__negatec
		.export __cplc
		.code

__negatec:
		dec	l
__cplc:
		ld	a,l
		cpl
		ld	l,a
		ret

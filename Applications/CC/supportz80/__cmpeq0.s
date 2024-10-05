		.export __cmpeq0
		.export __cmpeq0b
		.export __cmplteq0u
		.export __cmplteq0ub
		.code

__cmplteq0u:
__cmpeq0:
		ld	a,h
		or 	l
		jp	nz,__false
		jp	__true

__cmplteq0ub:
__cmpeq0b:
		ld	a,l
		or	a
		jp	nz,__false
		jp	__true

		.export __cmplteq0
		.code

__cmplteq0:
		ld	a,h
		or	a
		jp	m,__true
		jp	nz,__false
		ld	a,l
		or	a
		jp	nz,__false
		jp	__true

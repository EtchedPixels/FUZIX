		.export	__not
		.code

__not:
		ld	a,h
		or	l
		jp	z,__true
		jp	__false

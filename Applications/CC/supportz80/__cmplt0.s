		.export __cmplt0

		.code

__cmplt0:
		ld	a,h
		or	a
		jp	m,__true
		jp	__false


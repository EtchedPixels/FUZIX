		.export __cmpgt0
		.code

__cmpgt0:
		ld	a,h
		or	a
		jp	m,__false
		jp	nz,__true
		;	H was 0 so compare it with L
		cp	l
		jp	nz,__true
		jp	__false

		.export	__notl
		.code

__notl:
		ld	a,h
		or	l
		ld	hl,(__hireg)
		or	h
		or	l
		jp	z,__true
		jp	__false

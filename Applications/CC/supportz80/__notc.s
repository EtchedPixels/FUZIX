		.export	__notc
		.code

__notc:
		ld	a,l
		or	a
		jp	z,__true
		jp	__false

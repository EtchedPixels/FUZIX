		.export __cmpgteq0
		.code

__cmpgteq0:
		bit	7,h
		jp	nz,__false
		jp	__true

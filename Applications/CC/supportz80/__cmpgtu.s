		.export __cmpgtu
		.code

		; true if HL > DE
__cmpgtu:
		or	a
		sbc	hl,de
		jp	c,__false
		jp	nz,__true
		jp	__false

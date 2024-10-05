		.export __cmpgtu
		.export __cmpgtu0d
		.code

		; true if HL > DE
__cmpgtu0d:
		ld	d,0
__cmpgtu:
		or	a
		sbc	hl,de
		jp	c,__false
		jp	nz,__true
		jp	__false

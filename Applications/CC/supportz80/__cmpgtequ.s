		.export __cmpgtequ
		.export __cmpgtequ0d
		.export __cmpgtequb
		.code

		; true if HL >= DE

__cmpgtequ0d:
		ld	d,0
__cmpgtequ:
		or	a
		sbc	hl,de
		jp	c,__false
		jp	__true
__cmpgtequb:
		ld	a,l
		cp	e
		jp	c,__false
		jp	__true

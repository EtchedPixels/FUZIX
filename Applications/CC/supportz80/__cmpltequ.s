		.export __cmpltequ
		.export __cmpltequb
		.code

		; true if HL <= DE

__cmpltequ:
		or	a
		sbc	hl,de
		jp	c,__true
		jp	z,__true
		jp	__false
__cmpltequb:
		ld	a,l
		cp	e
		jp	c,__true
		jp	z,__true
		jp	__false

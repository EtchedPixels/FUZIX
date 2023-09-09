		.export __cmpltu
		.export __cmpltub
		.code

		; true if HL < DE
__cmpltu:
		or	a
		sbc	hl,de
		jp	c,__true
		jp	__false

__cmpltub:
		ld	a,l
		cp	e
		jp	c,__true
		jp	__false

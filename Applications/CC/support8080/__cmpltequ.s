		.export __cmpltequ
		.export __cmpltequb
		.setcpu 8080
		.code

		; true if HL <= DE

__cmpltequ:
		mov	a,h
		cmp	d
		jc	__true
		jnz	__false
__cmpltequb:
		mov	a,l
		cmp	e
		jz	__true
		jnc	__false
		jmp	__true

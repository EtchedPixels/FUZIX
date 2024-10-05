		.export __cmpgtequ
		.export __cmpgtequb
		.setcpu 8080
		.code

		; true if HL >= DE

__cmpgtequ:
		mov	a,h
		cmp	d
		jc	__false
		jnz	__true
__cmpgtequb:
		mov	a,l
		cmp	e
		jc	__false
		jmp	__true

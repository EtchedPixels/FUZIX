		.export __cmpltu
		.export __cmpltub
		.setcpu 8080
		.code

		; true if HL < DE
__cmpltu:
		mov	a,h
		cmp	d
		jc	__true
		jnz	__false
__cmpltub:
		mov	a,l
		cmp	e
		jc	__true
		jmp	__false

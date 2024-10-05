		.export __ccne

		.setcpu 8080
		.code

__ccne:		xchg
		pop	h
		xthl
		mov	a,l
		cmp	e
		jnz	__true
		mov	a,h
		cmp	d
		jnz	__true
		jmp	__false

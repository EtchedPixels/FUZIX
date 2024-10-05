		.export __cceq

		.setcpu 8080
		.code

__cceq:		xchg
		pop	h
		xthl
		mov	a,l
		cmp	e
		jnz	__false
		mov	a,h
		cmp	d
		jnz	__false
		jmp	__true

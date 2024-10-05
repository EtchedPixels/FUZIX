			.export __cmpgt0
			.setcpu 8080

			.code

__cmpgt0:
		mov	a,h
		ora	a
		jm	__false
		jnz	__true
		mov	a,l
		ora	a
		jnz	__true
		jmp	__false

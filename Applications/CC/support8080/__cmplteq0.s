			.export __cmplteq0
			.setcpu 8080

			.code

__cmplteq0:
		mov	a,h
		ora	a
		jm	__true
		jnz	__false
		mov	a,l
		ora	a
		jnz	__false
		jmp	__true


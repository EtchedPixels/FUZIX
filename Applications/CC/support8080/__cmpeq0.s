		.export __cmpeq0
		.export __cmpeq0b
		.export __cmplteq0u
		.export __cmplteq0ub
		.setcpu	8080
		.code

__cmplteq0u:
__cmpeq0:
		mov	a,h
		ora 	l
		jnz	__false
		jmp	__true

__cmplteq0ub:
__cmpeq0b:
		mov	a,l
		ora	a
		jnz	__false
		jmp	__true

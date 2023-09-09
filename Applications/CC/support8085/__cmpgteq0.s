			.export __cmpgteq0
			.setcpu 8080

			.code

__cmpgteq0:
		mov	a,h
		ora	a
		jm	__false
		jmp	__true


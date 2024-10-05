		.export	__not

		.setcpu 8080
		.code

__not:
		mov	a,h
		ora	l
		jz	__true
		jmp	__false

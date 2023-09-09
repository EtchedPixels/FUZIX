		.export	__notc

		.setcpu 8080
		.code

__notc:
		mov	a,l
		ora	a
		jz	__true
		jmp	__false

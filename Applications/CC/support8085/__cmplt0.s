			.export __cmplt0
			.setcpu 8080

			.code

__cmplt0:
		mov	a,h
		ora	a
		jm	__true
		jmp	__false


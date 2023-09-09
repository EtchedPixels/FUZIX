		.export __cmpneb

		.setcpu 8080
		.code

;
;	Tighter version with the other value in DE
;
__cmpneb:
		mov	a,l
		cmp	e
		jnz	__true
		jmp	__false

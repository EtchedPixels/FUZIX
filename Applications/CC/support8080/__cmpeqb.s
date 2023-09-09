		.export __cmpeqb

		.setcpu 8080
		.code

;
;	Tighter version with the other value in DE
;
__cmpeqb:
		mov	a,l
		cmp	e
		jnz	__false
		jmp	__true

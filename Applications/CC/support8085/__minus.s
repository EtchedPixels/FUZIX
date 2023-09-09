;
;	Compute TOS - HL -> HL
;
		.export __minus
		.export __minusu

		.setcpu 8085
		.code
__minus:
__minusu:
		xchg			; working register into DE
		pop	h		; return address
		xthl
		push	b		; save working BC
		mov	b,d
		mov	c,e
		dsub
		pop	b		; get B back
		ret

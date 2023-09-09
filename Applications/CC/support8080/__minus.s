;
;	Compute TOS - HL -> HL
;
		.export __minus
		.export __minusu

		.setcpu 8080
		.code
__minus:
__minusu:
		xchg			; working register into DE
		pop	h		; return address
		xthl
		mov	a,l
		sub	e
		mov	l,a
		mov	a,h
		sbb	d
		mov	h,a
		ret


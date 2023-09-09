;
;		True if TOS < HL
;
		.export __ccltequ

		.setcpu 8080

		.code

__ccltequ:
		xchg
		pop	h
		xthl
		mov	a,l
		sub	e
		mov	l,a
		mov	a,h
		sbb	d
		jc	__true
		ora	l
		jz	__true
		jmp	__false

;
;		True if TOS < HL
;
		.export __ccgtu

		.setcpu 8080

		.code

__ccgtu:
		xchg
		pop	h
		xthl
		mov	a,l
		sub	e
		mov	l,a
		mov	a,h
		sbb	d
		jc	__false
		ora	l
		; if C or Z then false
		jz	__false
		jmp	__true

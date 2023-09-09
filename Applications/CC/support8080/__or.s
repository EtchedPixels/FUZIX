;
;	Compute HL = TOS | HL
;
		.export __or
		.export __oru

		.setcpu 8080
		.code
__or:
__oru:
		xchg			; working register into DE
		pop	h		; return address
		xthl			; back on stack HL is now the bits
		mov	a,h
		ora	d
		mov	h,a
		mov	a,l
		ora	e
		mov	l,a
		ret

;
;	Compute HL = TOS ^ HL
;
		.export __xor
		.export __xoru

		.setcpu 8080
		.code
__xor:
__xoru:
		xchg			; working register into DE
		pop	h		; return address
		xthl
		mov	a,h
		xra	d
		mov	h,a
		mov	a,l
		xra	e
		mov	l,a
		ret

;
;	Compute HL = TOS & HL
;
		.export __orc
		.export __oruc

		.setcpu 8080
		.code
__orc:
__oruc:
		mov	a,l		; working register into A
		pop	h		; return address
		xthl
		ora	e
		mov	l,a
		ret

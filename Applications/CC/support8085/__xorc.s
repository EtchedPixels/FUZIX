;
;	Compute HL = TOS & HL
;
		.export __xorc
		.export __xoruc

		.setcpu 8080
		.code
__xorc:
__xoruc:
		mov	a,l		; working register into A
		pop	h		; return address
		xthl
		xra	l
		mov	l,a
		ret

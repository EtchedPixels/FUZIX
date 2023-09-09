;
;	Compute HL = TOS & HL
;
		.export __xorc
		.export __xoruc

		.code
__xorc:
__xoruc:
		ld	a,l		; working register into A
		pop	hl		; return address
		ex	(sp),hl
		xor	l
		ld	l,a
		ret

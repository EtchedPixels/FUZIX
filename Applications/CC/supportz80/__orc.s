;
;	Compute HL = TOS & HL
;
		.export __orc
		.export __oruc
		.code
__orc:
__oruc:
		ld	a,l		; working register into A
		pop	hl		; return address
		ex	(sp),hl
		or	e
		ld	l,a
		ret

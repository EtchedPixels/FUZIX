;
;	Compute HL = TOS ^ HL
;
		.export __xor
		.export __xoru

		.code
__xor:
__xoru:
		ex	de,hl		; working register into DE
		pop	hl		; return address
		ex	(sp),hl
		ld	a,h
		xor	d
		ld	h,a
		ld	a,l
		xor	e
		ld	l,a
		ret

;
;	Compute HL = TOS | HL
;
		.export __or
		.export __oru
		.code

__or:
__oru:
		ex	de,hl		; working register into DE
		pop	hl		; return address
		ex	(sp),hl		; back on stack HL is now the bits
		ld	a,h
		or	d
		ld	h,a
		ld	a,l
		or	e
		ld	l,a
		ret

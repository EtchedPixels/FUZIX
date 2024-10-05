;
;	Compute HL = TOS & HL
;
		.export __band

		.code
__band:
		ex	de,hl		; working register into DE
		pop	hl		; return address
		ex	(sp),hl		; top of stack is return, hl is value
		ld	a,h
		and	d
		ld	h,a
		ld	a,l
		and	e
		ld	l,a
		ret

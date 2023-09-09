;
;	Compute HL = TOS & HL
;
		.export __andc
		.export __anduc

		.code
__andc:
__anduc:
		ld	a,l		; working register into A
		pop	hl		; return address
		ex	(sp),hl		; TOS value into HL
		and	e
		ld	l,a
		ret

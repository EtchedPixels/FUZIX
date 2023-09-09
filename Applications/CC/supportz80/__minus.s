;
;	Compute TOS - HL -> HL
;
		.export __minus
		.export __minusu
		.code
__minus:
__minusu:
		ex	de,hl		; working register into DE
		pop	hl		; return address
		ex	(sp),hl
		or	a
		sbc	hl,de
		ret

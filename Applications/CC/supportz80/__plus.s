;
;	Compute TOS + HL -> HL
;
		.export __plus
		.export __plusu

		.code
__plus:
__plusu:
		ex	de,hl		; working register into DE
		pop	hl		; return address
		ex	(sp),hl
		add	hl,de
		ret


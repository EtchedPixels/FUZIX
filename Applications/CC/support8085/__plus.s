;
;	Compute TOS + HL -> HL
;
		.export __plus
		.export __plusu

		.setcpu 8080
		.code
__plus:
__plusu:
		xchg			; working register into DE
		pop	h		; return address
		xthl
		dad	d
		ret


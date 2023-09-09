		.export __cmpne
		.code
;
;	Tighter version with the other value in DE
;
__cmpne:
		or	a
		sbc	hl,de
		ret	z		; HL 0, Z set
		jp	__true

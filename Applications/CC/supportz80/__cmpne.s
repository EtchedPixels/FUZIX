		.export __cmpne
		.export __cmpne0d
		.code
;
;	Tighter version with the other value in DE
;
__cmpne0d:
		ld	d,0
__cmpne:
		or	a
		sbc	hl,de
		ret	z		; HL 0, Z set
		jp	__true

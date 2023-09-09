;
;		True if TOS >= HL
;
		.export __ccgtequ

		.setcpu 8080

		.code
;
__ccgtequ:
		xchg
		pop	h
		xthl
		mov	a,l
		sub	e
		mov	a,h
		sbb	d
		; If we carried it is less than
		cmc	; flip carry flag C is now set if true
		mvi	h,0
		mov	a,h
		adc	a		; 0 if original carried, 1 if not 
		mov	l,a
		ret

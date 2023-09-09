;
;		True if TOS < HL
;
		.export __ccltu

		.setcpu 8080

		.code

__ccltu:
		xchg
		pop	h
		xthl
		mov	a,l
		sub	e
		mov	a,h
		sbb	d
		mvi	h,0
		mov	a,h
		adc	a		; 1 if original carried, 0 if not
		mov	l,a
		ret

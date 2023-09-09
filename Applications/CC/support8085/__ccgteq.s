;
;		True if TOS >= HL
;
		.export __ccgteq

		.setcpu 8080

		.code
;
;	The 8080 doesn't have signed comparisons directly
;
;	The 8085 has K which might be worth using TODO
;
__ccgteq:
		xchg
		pop	h
		xthl
		mov	a,h
		xra	d
		jp	sign_same
		xra	d		; A is now H
		jm	__false
		jmp	__true
sign_same:
		; TOS is in HL, old HL in DE. Test HL >= DE (ie DE <=  HL)
		mov	a,e
		sub	l
		mov	e,a
		mov	a,d
		sbb	h
		jm	__true
		ora	e
		jz	__true
		jmp	__false


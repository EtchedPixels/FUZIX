;
;		True if HL <= DE
;
		.export __cmplteq
		.export __cmplteqb

		.setcpu 8080

		.code
;
;	The 8080 doesn't have signed comparisons directly
;
;	The 8085 has K which might be worth using TODO
__cmplteqb:
		mvi	h,0
		mov	d,h
__cmplteq:
		mov	a,h
		xra	d
		jp	sign_same
		xra	d		; A is now H
		jm	__true
		jmp	__false
sign_same:
		mov	a,e
		sub	l
		mov	l,a
		mov	a,d
		sbb	h
		jnc	__true
		ora	l
		jz	__true
		jmp	__false

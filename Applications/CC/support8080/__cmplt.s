;
;		True if HL < DE
;
		.export __cmplt
		.export __cmpltb

		.setcpu 8080

		.code
;
;	The 8080 doesn't have signed comparisons directly
;
;	The 8085 has K which might be worth using TODO
;
__cmpltb:
		mvi	h,0
		mov	d,h
__cmplt:
		mov	a,h
		xra	d
		jp	sign_same
		xra	d		; A is now H
		jm	__true
		jmp	__false
sign_same:
		mov	a,l
		sub	e
		mov	a,h
		sbb	d
		jc	__true
		jmp	__false

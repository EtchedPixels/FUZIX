;
;		True if HL > DE
;
		.export __cmpgt
		.export __cmpgtb

		.setcpu 8080

		.code
;
;	The 8080 doesn't have signed comparisons directly
;
;	The 8085 has K which might be worth using TODO
;
__cmpgtb:
                mvi	h,0
		mov	d,h
__cmpgt:
		mov	a,h
		xra	d
		jp	sign_same
		xra	d		; A is now H
		jm	__false
		jmp	__true
sign_same:
		mov	a,e
		sub	l
		mov	a,d
		sbb	h
		jc	__true
		jmp	__false

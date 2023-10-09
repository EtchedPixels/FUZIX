	.65c816
	.a16
	.i16

;
;	A X and @hireg etc are all eaten by the called function so we
;	just need sp and the return address
;
	.export __setjmp

__setjmp:
	ldx 2,y		; buffer pointer
	pla		; return address
	sta 0,x		; save PC
	pha		; put it back
	sty 2,x		; save SP
	rts

;
;		True if HL >= DE
;
		.export __cmpgteq
		.export __cmpgteqb

		.code
;
;	The 8080 doesn't have signed comparisons directly
;
;	The 8085 has K which might be worth using TODO
;
__cmpgteqb:
		ld	h,0
		ld	d,h
__cmpgteq:
		ld	a,h
		xor	d
		jp	p,sign_same
		xor	d		; A is now H
		jp	p,__true
		jp	__false
sign_same:
		; C is clear
		sbc	hl,de
		jp	c,__false
		jp	__true

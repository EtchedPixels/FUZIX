;
;		True if HL > DE
;
		.export __cmpgt
		.export __cmpgtb
		.code
;
;	The 8080 doesn't have signed comparisons directly
;
;	The 8085 has K which might be worth using TODO
;
__cmpgtb:
                ld	h,0
		ld	d,h
__cmpgt:
		ld	a,h
		xor	d
		jp	p,sign_same
		xor	d		; A is now H
		jp	m,__false
		jp	__true
sign_same:
		;  C is clear
		ex	de,hl
		sbc	hl,de
		jp	c,__true
		jp	__false

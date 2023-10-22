;
;		True if HL < DE
;
		.export __cmplt0d
		.export __cmplt
		.export __cmpltb
		.code
;
;	The 8080 doesn't have signed comparisons directly
;
;	The 8085 has K which might be worth using TODO
;
__cmpltb:
		ld	h,0
__cmplt0d:
		ld	d,0
__cmplt:
		ld	a,h
		xor	d
		jp	p,sign_same
		xor	d		; A is now H
		jp	m,__true
		jp	__false
sign_same:
		; C is clear
		sbc	hl,de
		jp	c,__true
		jp	__false

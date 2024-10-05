;
;		True if HL <= DE
;
		.export __cmplteq
		.export __cmplteq0d
		.export __cmplteqb

		.code
;
;	The 8080 doesn't have signed comparisons directly
;
;	The 8085 has K which might be worth using TODO
__cmplteqb:
		ld	h,0
__cmplteq0d:
		ld	d,0
__cmplteq:
		ld	a,h
		xor	d
		jp	p,sign_same
		xor	d		; A is now H
		jp	m,__true
		jp	__false
sign_same:
		; C is clear
		ex	de,hl
		sbc	hl,de
		jp	nc,__true
		jp	__false

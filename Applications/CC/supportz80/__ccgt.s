;
;		True if TOS > HL
;
		.export __ccgt
		.code
;
;	The 8080 doesn't have signed comparisons directly
;
;	The 8085 has K which might be worth using TODO
;
__ccgt:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		ld	a,h
		xor	d
		jp	p,sign_same
		xor	d		; A is now H
		jp	m,__false
		jp	__true
sign_same:
		; C is clear
		ex	de,hl
		sbc	hl,de
		jp	nc,__false
		jp	__true

;
;		True if TOS < HL
;
		.export __cclt
		.code
;
;	The 8080 doesn't have signed comparisons directly
;
;	The 8085 has K which might be worth using TODO
;
__cclt:
		ex	de,hl
		pop	hl
		ex	(sp),hl		; TOS in HL old HL in DE
		; Test HL < DE
		ld	a,h
		xor	d
		jp	p,sign_same
		xor	d		; A is now H
		jp	m,__true
		jp	__false
sign_same:
		; TOS is now in HL, old HL in DE, test  HL < DE
		; C is clear
		sbc	hl,de
		jp	c,__true
		jp	__false

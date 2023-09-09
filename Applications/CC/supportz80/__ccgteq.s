;
;		True if TOS >= HL
;
		.export __ccgteq
		.code
;
;	The 8080 doesn't have signed comparisons directly
;
__ccgteq:
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
		; TOS is in HL, old HL in DE. Test HL >= DE (ie DE <=  HL)
		; C will be clear
		ex	de,hl
		sbc	hl,de
		jp	m,__true
		or	e
		jp	z,__true
		jp	__false


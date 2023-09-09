;
;		True if TOS >= HL
;
		.export __ccgtequ
		.code
;
__ccgtequ:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		or	a
		sbc	hl,de
		; If we carried it is less than
		ccf	; flip carry flag C is now set if true
		ld	h,0
		ld	a,h
		adc	a,a		; 0 if original carried, 1 if not
		ld	l,a
		ret

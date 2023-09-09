;
;		True if TOS < HL
;
		.export __ccltu
		.code

__ccltu:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		or	a
		sbc	hl,de
		ld	hl,0
		ld	a,l
		adc	a,a		; 1 if original carried, 0 if not
		ld	l,a
		ret

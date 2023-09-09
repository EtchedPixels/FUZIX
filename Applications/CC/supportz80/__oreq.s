	;
;		TOS = lval of object HL = mask
;
		.export __oreq
		.export __orequ
		.code

__oreq:
__orequ:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		; Now DE is the mask and HL the pointer
		ld	a,(hl)
		or	e
		ld	(hl),a
		ld	e,a
		inc	hl
		ld	a,(hl)
		or	d
		ld	(hl),a
		ld	l,e
		ld	h,a
		ret

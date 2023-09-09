;
;		TOS = lval of object HL = mask
;
		.export __andeq
		.export __andequ

		.code
__andeq:
__andequ:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		; HL is now the pointer, DE the mask
		ld	a,(hl)
		and	e
		ld	(hl),a
		ld	e,a
		inc	hl
		ld	a,(hl)
		and	d
		ld	(hl),a
		ld	l,e
		ld	h,a
		ret

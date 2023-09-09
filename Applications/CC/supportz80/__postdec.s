;
;		TOS = lval of object HL = amount
;
		.export __postdec
		.code

__postdec:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		ld	a,(hl)
		ld	(__tmp),a
		sub	e
		ld	(hl),a
		inc	hl
		ld	a,(hl)
		ld	(__tmp+1),a
		sbc	a,d
		ld	(hl),a
                ld	hl,(__tmp)
		ret

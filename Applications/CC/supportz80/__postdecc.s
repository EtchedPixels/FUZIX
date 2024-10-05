;
;		TOS = lval of object HL = amount
;
		.export __postdecc
		.code

__postdecc:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		ld	a,(hl)
		ld	d,a		; save old value
		sub	e
		ld	(hl),a
		ld	l,d		; return old value
		ret

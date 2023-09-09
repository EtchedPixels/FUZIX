;
;		TOS = lval of object L = amount
;
		.export __minuseqc
		.export __minusequc
		.code

__minuseqc:
__minusequc:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		ld	a,(hl)
		sub	e
		ld	(hl),a
		ld	l,a
		ret


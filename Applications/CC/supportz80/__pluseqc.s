
;
;		TOS = lval of object L = amount
;
		.export __pluseqc
		.code

__pluseqc:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		ld	a,(hl)
		add	a,e
		ld	(hl),a
		ld	l,a
		ret

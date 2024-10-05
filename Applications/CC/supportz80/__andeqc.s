;
;		TOS = lval of object L = mask
;
		.export __andeqc

		.code
__andeqc:
		ex	de,hl
		pop	hl		; return address
		ex	(sp),hl		; swap it for the lval
		ld	a,e
		and	(hl)
		ld	(hl),a
		ld	l,a
		ret

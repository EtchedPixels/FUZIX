;
;		TOS = lval of object L = mask
;
		.export __oreqc
		.code

__oreqc:
		ex	de,hl
		pop	hl		; return address
		ex	(sp),hl		; swap it for the lval
		ld	a,e
		or	(hl)
		ld	(hl),a
		ld	l,a
		ret

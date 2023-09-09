;
;		TOS = lval of object L = mask
;
		.export __xoreqc
		.code
__xoreqc:
		ex	de,hl
		pop	hl		; return address
		ex	(sp),hl		; swap it for the lval
		ld	a,e
		xor	(hl)
		ld	(hl),a
		ld	l,a
		ret

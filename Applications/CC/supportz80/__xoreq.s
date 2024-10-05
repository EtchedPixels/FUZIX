;
;		TOS = lval of object HL = mask
;
		.export __xoreq
		.export __xorequ

		.code
__xoreq:
__xorequ:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		; Mask now in DE pointer in HL
		ld	a,(hl)
		xor	e
		ld	(hl),a		; xor byte and store
		ld	e,a
		inc	hl		; high byte
		ld	a,(hl)
		xor	d
		ld	(hl),a		; same again
		ld	l,e		; recover value for caller
		ld	h,a
		ret

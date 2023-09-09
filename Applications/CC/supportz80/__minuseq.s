;
;		TOS = lval of object HL = amount
;
		.export __minuseq
		.code

__minuseq:
		ex	de,hl	; DE is now the amount
		pop	hl	; return
		ex	(sp),hl	; return in place of TOS ptr

		; HL = ptr DE = value

		ld	a,(hl)
		sub	e
		ld	(hl),a
		ld	e,a	; Save result into DE
		inc	hl
		ld	a,(hl)
		sbc	a,d
		ld	(hl),a
		ld	d,a	; Result now in DE also
		ex	de,hl	; into HL for return
		ret

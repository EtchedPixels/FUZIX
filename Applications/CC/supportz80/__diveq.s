;
;		(TOS) /= HL
;

		.export __diveq
		.code

__diveq:
		ex	de,hl		; save value in DE
		pop	hl		; return address
		ex	(sp),hl		; swap with TOS lval
		; Now we are doing (HL) * DE
		push	de		; save value
		ld	e,(hl)
		inc	hl
		ld	d,(hl)		; DE is now (TOS)
		dec	hl
		ex	(sp),hl	; swap HL - TOS lval with top of stack (DE for division)
		; We are now doing HL / DE and the address we want is TOS
		ex	de,hl
		call	__divde
		; Return is in HL
		ex	de,hl
		pop	hl
		ld	(hl),e	; Save return
		inc	hl
		ld	(hl),d
		ex	de,hl		; Result back in HL
		ret

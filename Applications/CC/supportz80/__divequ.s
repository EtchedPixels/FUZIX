;
;		(TOS) /= HL
;

		.export __divequ
		.code

__divequ:
		ex	de,hl		; value to divide by in DE
		pop	hl
		ex	(sp),hl
		; Now we are doing (HL) / DE
		push	de
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		dec	hl
		ex	(sp),hl	; swap address with stacked value
		ex	de,hl
		; We are now doing HL / DE and the address we want is TOS
		call	__divdeu
		; Return is in HL
		ex	de,hl
		pop	hl
		ld	(hl),e
		inc	hl
		ld	(hl),d
		ex	de,hl
		ret

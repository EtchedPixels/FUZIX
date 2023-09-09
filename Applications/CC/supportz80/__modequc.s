;
;		(TOS) /= L
;

		.export __remequc
		.code

__remequc:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		; Now we are doing (HL) * DE
		push	de
		ld	e,(hl)
		ex	(sp),hl	; swap address with stacked value
		ex	de,hl	; swap them back as we divide by DE
		; We are now doing HL / DE and the address we want is TOS
		ld	h,0
		ld	d,h
		call	__remde
		; Return is in HL
		ex	de,hl
		pop	hl
		ld	(hl),e
		ex	de,hl
		ret

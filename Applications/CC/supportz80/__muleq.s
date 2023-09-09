;
;		(TOS) *= HL
;
		.export __muleq
		.code

__muleq:
		ex	de,hl
		pop	hl
		ex	(sp),hl
		; Now we are doing (HL) * DE
		push	de
		ld	e,(hl)
		inc	hl
		ld	d,(hl)
		dec	hl
		ex	(sp),hl
		; We are now doing HL * DE and the address we want is TOS
		call	__mulde
		; Return is in HL
		ex	de,hl
		pop	hl
		ld	(hl),e
		inc	hl
		ld	(hl),d
		ex	de,hl
		ret

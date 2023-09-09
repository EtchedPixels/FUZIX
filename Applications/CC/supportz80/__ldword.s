;
;	Load word from further off stack
;
		.export __ldword
		.code

__ldword:
		pop	hl	; Get byte following call
		ld	e,(hl)
		inc	hl
		ld	d,0
		push	hl	; Return to byte after info byte
		ex	de,hl	; HL is now the offset
		add	hl,sp	; Add to SP
		ld	a,(hl)	; Load into HL
		inc	hl
		ld	h,(hl)
		ld	l,a
		ret


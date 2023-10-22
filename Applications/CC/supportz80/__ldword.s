;
;	Load word from further off stack
;
		.export __ldword
		.code

__ldword:
		pop	hl	; Get byte following call
		ld	a,(hl)
		inc	hl
		push	hl	; Return to byte after info byte
		ld	l,a
		ld	h,0
		add	hl,sp	; Add to SP
		ld	a,(hl)	; Load into HL
		inc	hl
		ld	h,(hl)
		ld	l,a
		ret


;
;	Load word from further off stack
;
		.export __ldword

		.setcpu 8080
		.code

__ldword:
	pop	h		; Get byte following call
	mov	e,m
	inx	h
	mvi	d,0
	push	h		; Return to byte after info byte
	xchg			; HL is now the offset
	dad	sp		; Add to SP
	mov	a,m		; Load into HL
	inx	h
	mov	h,m
	mov	l,a
	ret


;
;	Load word from further off stack
;
		.export __ldword

		.setcpu 8080
		.code

__ldword:
	pop	h		; Get byte following call
	mov	a,m
	inx	h
	push	h		; Return to byte after info byte
	mvi	h,0
	mov	l,a		; HL is now the offset
	dad	sp		; Add to SP
	mov	a,m		; Load into HL
	inx	h
	mov	h,m
	mov	l,a
	ret


;
;	Not a lot to do because we don't have to save much. We save BC ready
;	for future use
;

		.export __setjmp
		.setcpu 8085
		.code

__setjmp:
	ldsi	2
	lhlx			; address of buffer into HL
	xchg			; to DE
	lxi	h,0
	dad	sp		; get SP
	shlx			; save SP in buffeer
	inx	d
	inx	d
	pop	h		; get return address
	shlx			; save into buffer
	push	h		; put it back
	inx	d
	inx	d
	xchg			; buffer back into HL
	mov	m,c		; Save BC
	inx	h
	mov	m,b
	lxi	h,0		; Return 0
	ret

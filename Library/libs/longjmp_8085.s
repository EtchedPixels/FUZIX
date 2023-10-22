;
;	Not a lot to do because we don't have to save much. We save BC ready
;	for future use
;

		.export _longjmp
		.setcpu 8085
		.code

_longjmp:
	ldsi	4
	lhlx			; return code
	mov	a,l
	ora	h
	jnz	retok
	inx	h
retok:
	mov	c,l
	mov	b,h		; save return code
	ldsi	2
	lhlx			; address of buffer 
	xchg			; to DE
	lhlx			; get value for SP restore
	inx	h		; discard old return address
	inx	h
	sphl			; restore SP
	inx	d
	inx	d
	lhlx			; return address
	push	h		; put it back
	push	b		; return code
	inx	d
	inx	d
	xchg			; buffer back into HL
	mov	c,m		; restore BC
	inx	h
	mov	b,m
	pop	h		; return code
	ret

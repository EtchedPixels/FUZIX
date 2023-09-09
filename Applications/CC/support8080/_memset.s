;
;	Memset
;
		.export _memset
		.setcpu 8080
		.code
_memset:
	push	b
	lxi	h,4		; Allow for the push of B
	dad	sp
	mov	e,m
	inx	h
	mov	d,m		; Pointer
	push	d		; Return is the passed pointer
	inx	h
	mov	a,m		; fill byte
	inx	h		; skip fill high
	inx	h
	mov	c,m
	inx	h
	mov	b,m		; length into BC

	mov	l,a		; We need to free up A for the loop check
	xchg			; now have HL as the pointer and E as the fill byte
	jmp	loopin

loop:
	mov	m,e
	inx	h
	dcx	b
loopin:
	mov	a,b
	ora	c
	jnz	loop
	pop	h		; Address passed in
	pop	b		; Restore B
	ret


		.export __muldec
		.setcpu 8080
		.code

;
;	L * A into HL
;


__muldec:
	push	b
	mov	e,a
	mov	d,l		; now D * E
	lxi	h,0		; into HL
	mvi	b,8
next:
	mov	a,d
	rar
	mov	d,a
	jnc	noadd
	mov	a,h
	add	e
	mov	h,a
noadd:	mov	a,h
	rar
	mov	h,a
	mov	a,l
	rar
	mov	l,a
	dcr	b
	jnz	next
	pop	b
	ret


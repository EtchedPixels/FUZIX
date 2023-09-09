;
;	memcpy
;
		.export _memcpy
		.setcpu 8080
		.code
_memcpy:
	push	b
	lxi	h,9
	dad	sp
	mov	b,m
	dcx	h
	mov	c,m
	dcx	h
	mov	d,m	; Source
	dcx	h
	mov	e,m
	dcx	h
	mov	a,m	; Destination
	dcx	h
	mov	l,m
	mov	h,a

	mov	a,c
	ora	b
	jz	done

	dcx	b
	push	h
loop:
	ldax	d
	inx	d
	mov	m,a
	inx	h
	dcx	b
	mov	a,b
	ora	c
	jnz	loop
	pop	h

done:
	pop	b
	ret

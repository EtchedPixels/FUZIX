;
;	memcpy
;
		.export _memcpy
		.setcpu 8085
		.code
_memcpy:
	push	b
	ldsi	9	; High byte of count
	xchg
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
	jnk	loop
	pop	h

done:
	pop	b
	ret

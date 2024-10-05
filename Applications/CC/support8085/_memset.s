;
;	Memset
;
		.export _memset
		.setcpu 8085
		.code
_memset:
	push	b
	ldsi	7
	ldax	d		; fill byte
	ldsi	8		; count
	lhlx
	mov	b,h
	mov	c,l
	ldsi	4		; pointer
	lhlx


	mov	e,a
	mov	a,c
	ora	b
	jz	done

	dcx	b
	push	h
loop:
	mov	m,e
	inx	h
	dcx	b
	jnk	loop
	pop	h

done:
	pop	b
	ret

;
;		Long or, values are on stack and in hireg:h
;
		.export __bandl
		.setcpu	8080
		.code

__bandl:
	xchg			; pointer into de
	lxi	h,2
	dad	sp		; so m is the memory pointer, de the value
	mov	a,m
	ana	e
	mov	e,a
	inx	h
	mov	a,m
	ana	d
	mov	d,a
	inx	h
	push	d
	xchg
	lhld	__hireg
	xchg
	mov	a,m
	ana	e
	mov	e,a
	inx	h
	mov	a,m
	ana	d
	mov	d,a
	xchg
	shld	__hireg
	pop	d		; result
	pop	h
	shld	__retaddr
	pop	h
	pop	h
	xchg
	jmp	__ret

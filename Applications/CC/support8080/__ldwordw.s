;
;	Load word from further off stack
;
		.export __ldwordw

		.setcpu 8080
		.code

__ldwordw:
	pop	h
	mov	e,m
	inx	h
	mov	d,m
	inx	h
	push	h
	dad	sp
	mov	a,m
	inx	h
	mov	h,m
	mov	l,a
	ret


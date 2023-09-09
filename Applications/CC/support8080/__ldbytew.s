;
;	Load byte from further off stack
;
		.export __ldbytew

		.setcpu 8080
		.code

__ldbytew:
	pop	h
	mov	e,m
	inx	h
	mov	d,m
	inx	h
	push	h
	dad	sp
	mov	l,m
	ret


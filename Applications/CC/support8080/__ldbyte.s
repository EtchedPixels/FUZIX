;
;	Load byte from 8bit range of stack
;
		.export __ldbyte

		.setcpu 8080
		.code

__ldbyte:
	pop	h
	mov	e,m
	inx	h
	mvi	d,0
	push	h
	dad	sp
	mov	l,m
	ret

;
;	Load byte from 8bit range of stack
;
		.export __ldbyte

		.setcpu 8080
		.code

__ldbyte:
	pop	h
	mov	a,m
	inx	h
	push	h
	mvi	h,0
	mov	l,a
	dad	sp
	mov	l,m
	ret

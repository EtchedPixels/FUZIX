;
;	HL = lval
;
	.export __pluseq1
	.export __pluseq2
	.export __pluseq1d
	.export __pluseq2d

	.setcpu 8080
	.code

__pluseq1d:
	xchg
 __pluseq1:
	mov	e,m
	inx	h
	mov	d,m
	inx	d
	mov	m,d
	dcx	h
	mov	m,e
	xchg
	ret

__pluseq2d:
	xchg
__pluseq2:
	mov	e,m
	inx	h
	mov	d,m
	inx	d
	inx	d
	mov	m,d
	dcx	h
	mov	m,e
	xchg
	ret

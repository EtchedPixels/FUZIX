;
;	HL = lval
;
	.export __minuseq1
	.export __minuseq2
	.export __minuseq1d
	.export __minuseq2d

	.setcpu 8080
	.code

__minuseq1d:
	xchg
__minuseq1:
	mov	e,m
	inx	h
	mov	d,m
	dcx	d
	mov	m,d
	dcx	h
	mov	m,e
	xchg
	ret

__minuseq2d:
	xchg
__minuseq2:
	mov	e,m
	inx	h
	mov	d,m
	dcx	d
	dcx	d
	mov	m,d
	dcx	h
	mov	m,e
	xchg
	ret

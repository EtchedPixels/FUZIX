	.export __postinc1
	.export __postinc2
	.export __postinc1d
	.export __postinc2d

	.setcpu 8080
	.code

__postinc1d:
	xchg
__postinc1:
	mov	e,m
	inx	h
	mov	d,m
	inx	d
	mov	m,d
	dcx	h
	mov	m,e
	xchg
	dcx	h
	ret

__postinc2d:
	xchg
__postinc2:
	mov	e,m
	inx	h
	mov	d,m
	inx	d
	inx	d
	mov	m,d
	dcx	h
	mov	m,e
	xchg
	dcx	h
	dcx	h
	ret

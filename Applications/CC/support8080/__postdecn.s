	.export __postdec1
	.export __postdec2
	.export __postdec1d
	.export __postdec2d

	.setcpu 8080
	.code

__postdec1d:
	xchg
__postdec1:
	mov	e,m
	inx	h
	mov	d,m
	dcx	d
	mov	m,d
	dcx	h
	mov	m,e
	xchg
	inx	h
	ret

__postdec2d:
	xchg
__postdec2:
	mov	e,m
	inx	h
	mov	d,m
	dcx	d
	dcx	d
	mov	m,d
	dcx	h
	mov	m,e
	xchg
	inx	h
	inx	h
	ret

;
;		(HL) &= DE
;
			.export __xoreqde
			.setcpu 8080
			.code

__xoreqde:
	mov	a,m
	xra	e
	mov	m,a
	mov	e,a
	inx	h
	mov	a,m
	xra	d
	mov	m,a
	mov	d,a
	xchg
	ret


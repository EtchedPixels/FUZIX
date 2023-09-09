;
;		(HL) &= DE
;
			.export __oreqde
			.setcpu 8080
			.code

__oreqde:
	mov	a,m
	ora	e
	mov	m,a
	mov	e,a
	inx	h
	mov	a,m
	ora	d
	mov	m,a
	mov	d,a
	xchg
	ret


;
;		(HL) &= DE
;
			.export __andeqde
			.setcpu 8080
			.code

__andeqde:
	mov	a,m
	ana	e
	mov	m,a
	mov	e,a
	inx	h
	mov	a,m
	ana	d
	mov	m,a
	mov	d,a
	xchg
	ret


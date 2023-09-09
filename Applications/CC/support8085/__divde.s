;
;	Signed division. Do this via unsigned
;
		.setcpu 8080
		.export __div
		.export __divde
		.export __rem
		.export __remde

__div:
	xchg
	pop	h
	xthl
	call	__divde
	ret

__divde:
	push	b
	mvi	c,0
	call	signfix
	xchg
	call	signfix
	xchg
	;	C tells us if we need to negate

	call	__divdeu

	mov	a,c
	rar
	cc	negate
	pop	b
	ret

__rem:
	xchg
	pop	h
	xthl
__remde:
	push	b
	xchg
	call	signfix
	xchg
	mvi	c,0
	call	signfix
	;	C tells us if we need to negate

	call	__remdeu

	mov	a,c
	rar
	cc	negate
	pop	b
	ret

;	Turn HL positive, xor a with one if was
signfix:
	mov	a,h
	ora	a
	rp
negate:
	cma
	mov	h,a
	mov	a,l
	cma
	mov	l,a
	inx	h
	inr	c
	ret

;
;	TOS is the lval, hl is the shift amount
;
;
	.export __shreq
	.setcpu 8080
	.code

__shreq:
	xchg		; shift into de
	pop	h
	xthl		; pointer into hl

	mov	a,e	; save shift value
	xchg
	mov	e,m
	inx	h
	mov	d,m

	; No work to do
	ani	15
	rz

	push	b
	mov	c,a

	mov	a,d
	ora	a
	jm	sh1
shuffle:
	ora	a
	mov	a,d
	rar
	mov	d,a
	mov	a,e
	rar
	mov	e,a
	dcr	c
	jnz	shuffle
shdone:
	pop	b
	mov	m,d
	dcx	h
	mov	m,e
	xchg
	ret

sh1:
	stc
	mov	a,d
	rar
	mov	d,a
	mov	a,e
	rar
	mov	e,a
	dcr	c
	jnz	sh1
	jmp	shdone

;
;	TOS is the lval, hl is the shift amount
;
;
	.export __shrequ
	.setcpu 8080
	.code

__shrequ:
	xchg		; shift into de
	pop	h
	xthl		; pointer into hl

	mov	a,e	; save shift value

	mov	e,m
	inx	h
	mov	d,m	; DE is value HL is ptr

	; No work to do
	ani	15
	jz	nowork

	cpi	8
	jc	nobyte

	mov	e,d	; Shift 8 bits in one go
	mvi	d,0

	sui	8
nobyte:
	rz
	push	b
	mov	c,a
shuffle:
	mov	a,d
	ora	a
	rar		; Shift arithmetic (carry set up correctly)
	mov	d,a
	mov	a,e
	rar
	mov	e,a

	dcr	c
	jnz	shuffle

	mov	m,d	; Store back into HL
	dcx	h
	mov	m,e

	pop	b	; Recover B
nowork:
	xchg		; Value is the return
	ret

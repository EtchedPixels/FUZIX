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

	mov	e,m	; Value into DE, pointer is HL
	inx	h
	mov	d,m

	; No work to do
	ani	15
	jz	nowork

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
	mov	m,d	; Store result
	dcx	h
	mov	m,e
nowork:			; Value into HL, and done
	xchg
	ret

;
;	Same work loop but for negative numbers
;
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

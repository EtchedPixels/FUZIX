;
;	TOS is the lval, hl is the shift amount
;
;
		.export __shrequ
		.setcpu 8085
		.code

__shrequ:
	xchg		; shift into de
	pop	h
	xthl		; pointer into hl

	mov	a,e	; save shift value
	xchg
	lhlx		; pointer in de val amount in hl

	; No work to do
	ani	15
	rz

	cpi	8
	jc	nobyte

	mov	l,h
	mvi	h,0

	sui	8
nobyte:
	rz
	push	b
	mov	c,a
shuffle:
	mov	a,h
	ora	a
	rar		; Shift arithmetic (carry set up correctly)
	mov	h,a
	mov	a,l
	rar
	mov	l,a

	dcr	c
	jnz	shuffle

	pop	b
	shlx
	ret

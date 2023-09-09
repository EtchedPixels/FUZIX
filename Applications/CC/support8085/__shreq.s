;
;	TOS is the lval, hl is the shift amount
;
;
		.export __shreq
		.setcpu 8085
		.code

__shreq:
	xchg		; shift into de
	pop	h
	xthl		; pointer into hl

	mov	a,e	; save shift value
	xchg
	lhlx		; pointer in de val amount in hl

	; No work to do
	ani	15
	rz

	rz
	push	b
	mov	c,a
shuffle:
	arhl
	dcr	c
	jnz	shuffle

	pop	b
	shlx
	ret

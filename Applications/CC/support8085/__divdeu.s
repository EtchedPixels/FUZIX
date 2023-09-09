;
;			HL / DE	unsigned
;
		.export __divu
		.export __divdeu
		.export __remu
		.export __remdeu

		.setcpu	8080
		.code

__divu:
	xchg
	pop	h
	xthl
	call	__divdeu
	ret

__divdeu:
	call	__remdeu
	xchg
	ret


__remu:
	xchg
	pop	h
	xthl
__remdeu:
	push	b

	;	HL dividend, DE divisor
	xchg

	;	DE is now the dividend
	;	Negate HL into BC, so we can use dad to 16bit subtract

	mov	a,l
	cma
	mov	c,a
	mov	a,h
	cma
	mov	b,a
	inx	b

	;	16 iterations, clear working register

	lxi	h,0
	mvi	a,16

divloop:
	push	psw

	;	HLDE <<= 1
	dad	h
	xchg
	dad	h
	xchg
	jnc	nocopy
	inx	h		; safe to inr l ?
nocopy:

	push	h		; save remainder to stack
	dad	b		; subtract
	jnc	bigenough

	xthl			; swap remainder with result
	inx	d		; set the low bit (is it safe to inr e ?

bigenough:
	pop	h		; remove remainder/result from stack
	pop	psw		; recover count
	dcr	a		; iterate
	jnz	divloop

	;	DE = result, HL remainder
	pop	b
	ret

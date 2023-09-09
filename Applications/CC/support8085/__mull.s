;
;	It's difficult to make this re-entrant, if not nigh impossible on
;	8080. Even on 8085 the combination of ldsi/lhlx uses both de and hl
;	so doesn't really work out well
;
;		working = working * TOS
;
;	TODO: fast path 0 bytes ?
;
			.setcpu	8080
			.export __mull
			.export __muleql
			.export __copy4
			.code

__mull:
	shld	__tmp		; hireg:tmp is now one half of the sum
	pop	d		; d is the return address
	pop	h
	shld	__tmp2		; tmp2 holds the other
	pop	h
	shld	__tmp2 + 2
	push	d		; return address back

__domull:
	push	b		; save BC


	lxi	h,0
	shld	__tmp3		; tmp3 our working result
	shld	__tmp3+2
	lxi b,	0

nextbyte:
	lxi h,	__tmp		; work through tmp into hireg
	dad	b		; at this point B is 0 and C is byte count
	mov	a,m		; get next byte of multiplier
	mvi	b,8		; work through the byte

nextbit:
	rar
	jnc	noadd
	lhld	__tmp2		; 32bit add of the product
	xchg
	lhld	__tmp3
	dad	d
	shld	__tmp3
	lhld	__tmp2+2
	jnc	nocarry
	inr	l		; we know l is even so this can't carry
nocarry:
	xchg
	lhld	__tmp3+2
	dad	d
	shld	__tmp3+2
noadd:
	lhld	__tmp2		; 32bit left shift using dad
	dad	h
	shld	__tmp2
	lhld	__tmp2+2
	jnc	noshiftc
	dad	h
	inr	l		; we know l is even so this can't carry
	jmp	shifted
noshiftc:
	dad	h
shifted:
	shld	__tmp2+2

	; Complete the byte
	dcr	b
	jnz	nextbit

	; Now move on to the next word
	inr	c
	mov	a,c
	cpi	4
	jnz	nextbyte

	; At this point tmp3 holds the 32bit result
	lhld __tmp3+2
	shld __hireg
	lhld __tmp3
	pop	b
	ret

__muleql:
	shld	__tmp		; working into hireg:tmp
	pop	h		; return address
	xthl			; swap back in for lval pointer
	push	h
	lxi	d,__tmp2	; copy lval into tmp2
	call	__copy4
	call	__domull	; result is now in hireg;tmp
	pop	d
	lxi	h,__tmp
	call	__copy4		; stick it back in the register
	lhld	__tmp		; set up HL correctly for return
	ret

__copy4:			; copy 4 bytes from M to (D)
	mov	a,m
	stax	d
	inx	h
	inx	d
	mov	a,m
	stax	d
	inx	h
	inx	d
	mov	a,m
	stax	d
	inx	h
	inx	d
	mov	a,m
	stax	d
	ret

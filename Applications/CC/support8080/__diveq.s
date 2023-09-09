;
;		(TOS) /= HL
;

			.export __diveq
			.setcpu 8080
			.code
__diveq:
	xchg		; save value in DE
	pop	h	; return address
	xthl		; swap with TOS lval
	; Now we are doing (HL) * DE
	push	d	; save value
	mov	e,m
	inx	h
	mov	d,m	; DE is now (TOS)
	dcx	h
	xthl	; swap HL - TOS lval with top of stack (DE for division)
	; We are now doing HL / DE and the address we want is TOS
	xchg
	call __divde
	; Return is in HL
	xchg
	pop	h
	mov	m,e	; Save return
	inx	h
	mov	m,d
	xchg		; Result back in HL
	ret

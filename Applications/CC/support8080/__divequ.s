;
;		(TOS) /= HL
;

			.export __divequ
			.setcpu 8080
			.code
__divequ:
	xchg		; value to divide by in DE
	pop	h
	xthl
	; Now we are doing (HL) / DE
	push	d
	mov	e,m
	inx	h
	mov	d,m
	dcx	h
	xthl	; swap address with stacked value
	xchg
	; We are now doing HL / DE and the address we want is TOS
	call __divdeu
	; Return is in HL
	xchg
	pop	h
	mov	m,e
	inx	h
	mov	m,d
	xchg
	ret

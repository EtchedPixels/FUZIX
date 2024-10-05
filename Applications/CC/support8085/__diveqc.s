;
;		(TOS) /= L
;

			.export __diveqc
			.setcpu 8080
			.code
__diveqc:
	xchg
	pop	h
	xthl
	; Now we are doing (HL) / E
	push	h
	mov	l,m
	; We are now doing HL / DE and the address we want is TOS
	call	__sex
	xchg
	call	__sex
	xchg
	call __divdeu
	; Return is in HL
	pop	d
	mov	a,l
	stax	d
	ret

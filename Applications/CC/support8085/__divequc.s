;
;		(TOS) /= L
;

			.export __divequc
			.setcpu 8080
			.code
__divequc:
	xchg
	pop	h
	xthl
	; Now we are doing (HL) / E
	push	h
	mov	l,m
	; We are now doing HL / DE and the address we want is TOS
	call __divdeu
	; Return is in HL
	pop	d
	mov	a,l
	stax	d
	ret

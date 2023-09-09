;
;		(TOS) /= L
;

			.export __remeqc
			.setcpu 8085
			.code
__remeqc:
	xchg
	pop	h
	xthl
	; Now we are doing (HL) * DE
	push	d
	xchg
	lhlx	; get TOS into HL
	xchg
	xthl	; swap address with stacked value
	xchg	; swap them back as we divide by DE
	; We are now doing HL / DE and the address we want is TOS
	call	__sex
	xchg
	call	__sex
	xchg
	call	__remde
	; Return is in HL
	pop	d
	mov	a,l
	stax	d
	ret

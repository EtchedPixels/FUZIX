;
;		(TOS) /= L
;

			.export __remequc
			.setcpu 8085
			.code
__remequc:
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
	mvi	h,0
	mov	d,h
	call __remde
	; Return is in HL
	pop	d
	mov	a,l
	stax	d
	ret

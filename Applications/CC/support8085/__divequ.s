;
;		(TOS) /= HL
;

			.export __divequ
			.setcpu 8085
			.code
__divequ:
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
	call __divdeu
	; Return is in HL
	pop	d
	shlx
	ret

;
;		(TOS) *= HL
;

			.export __muleq
			.setcpu 8085
			.code
__muleq:
	xchg
	pop	h
	xthl
	; Now we are doing (HL) * DE
	push	d
	xchg
	lhlx
	xchg
	xthl
	; We are now doing HL * DE and the address we want is TOS
	call __mulde
	; Return is in HL
	pop	d
	shlx
	ret

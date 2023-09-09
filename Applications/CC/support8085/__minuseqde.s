;
;	Add de to (HL), return the result
;
			.export __minuseqde
			.setcpu 8085
			.code
__minuseqde:
	push	b
	mov	b,d	; save addition value into BC
	mov	c,e
	xchg		; we want the address in DE for an 8085
	lhlx		; get the value
	dsub
	shlx		; save it back
	pop	b	; restore BC
	ret

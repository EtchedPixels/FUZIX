;
;	Add de to (HL), return the result
;
			.export __pluseqde
			.setcpu 8085
			.code
__pluseqde:
	push	b
	mov	b,d	; save addition value into BC
	mov	c,e
	xchg		; we want the address in DE for an 8085
	lhlx		; get the value
	dad	b	; do the math
	shlx		; save it back
	pop	b	; restore BC
	ret

			.export __ccltul
			.setcpu 8080
			.code

__ccltul:
	call	__cmpulws
	pop	h		; return address
	pop	d		; value
	pop	d
	push	h
	jz	__false
	jnc	__true
	jmp	__false


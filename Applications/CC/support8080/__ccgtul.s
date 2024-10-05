			.export __ccgtul
			.setcpu 8080
			.code

__ccgtul:
	call	__cmpulws
	pop	h		; return address
	pop	d		; value
	pop	d
	push	h
	jnc	__false
	jmp	__true

			.export __ccltequl
			.setcpu 8080
			.code

__ccltequl:
	call	__cmpulws
	pop	h		; return address
	pop	d		; value
	pop	d
	push	h
	jnc	__true
	jmp	__false


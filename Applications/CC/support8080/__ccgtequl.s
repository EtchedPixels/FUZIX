			.export __ccgtequl
			.setcpu 8080
			.code

__ccgtequl:
	call	__cmpulws
	pop	h		; return address
	pop	d		; value
	pop	d
	push	h
	jz	__true
	jnc	__false
	jmp	__true



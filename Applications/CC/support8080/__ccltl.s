			.export __ccltl
			.setcpu 8080
			.code

__ccltl:
	call	__cmplws
	pop	h		; return address
	pop	d		; value
	pop	d
	push	h
	jz	__false
	jnc	__true
	jmp	__false


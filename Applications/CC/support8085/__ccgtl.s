			.export __ccgtl
			.setcpu 8080
			.code

__ccgtl:
	call	__cmplws
	pop	h		; return address
	pop	d		; value
	pop	d
	push	h
	jnc	__false
	jmp	__true

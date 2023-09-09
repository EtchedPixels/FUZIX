			.export __cclteql
			.setcpu 8080
			.code

__cclteql:
	call	__cmplws
	pop	h		; return address
	pop	d		; value
	pop	d
	push	h
	jnc	__true
	jmp	__false


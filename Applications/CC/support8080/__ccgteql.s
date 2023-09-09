			.export __ccgteql
			.setcpu 8080
			.code

__ccgteql:
	call	__cmplws
	pop	h		; return address
	pop	d		; value
	pop	d
	push	h
	jz	__true
	jnc	__false
	jmp	__true



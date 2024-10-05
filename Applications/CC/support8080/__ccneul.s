			.export __ccneul
			.export __ccnel
			.setcpu 8080
			.code

__ccnel:
__ccneul:
	call	__cmpulws
	pop	h		; return address
	pop	d		; value
	pop	d
	push	h
	jz	__false
	jmp	__true



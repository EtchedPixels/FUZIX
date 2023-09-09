		.export __ccneul
		.export __ccnel
		.code

__ccnel:
__ccneul:
		call	__cmpulws
		pop	hl		; return address
		pop	de		; value
		pop	de
		push	hl
		jp	z,__false
		jp	__true



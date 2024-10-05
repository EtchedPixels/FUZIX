		.export __ccgtul
		.code

__ccgtul:
		call	__cmpulws
		pop	hl		; return address
		pop	de		; value
		pop	de
		push	hl
		jp	nc,__false
		jp	__true

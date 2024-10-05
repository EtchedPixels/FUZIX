		.export __ccltul
		.code

__ccltul:
		call	__cmpulws
		pop	hl		; return address
		pop	de		; value
		pop	de
		push	hl
		jp	z,__false
		jp	nc,__true
		jp	__false


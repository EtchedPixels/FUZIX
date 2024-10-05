		.export __ccltequl
		.code

__ccltequl:
		call	__cmpulws
		pop	hl		; return address
		pop	de		; value
		pop	de
		push	hl
		jp	nc,__true
		jp	__false


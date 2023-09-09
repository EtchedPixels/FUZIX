		.export __ccgtequl
		.code

__ccgtequl:
		call	__cmpulws
		pop	hl		; return address
		pop	de		; value
		pop	de
		push	hl
		jp	z,__true
		jp	nc,__false
		jp	__true

		.export __ccequl
		.export __cceql
		.code

__cceql:
__ccequl:
		call	__cmpulws
		pop	hl		; return address
		pop	de		; value
		pop	de
		push	hl
		jp	z,__true
		jp	__false


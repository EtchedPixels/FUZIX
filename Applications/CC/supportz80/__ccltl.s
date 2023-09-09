		.export __ccltl
		.code

__ccltl:
		call	__cmplws
		pop	hl		; return address
		pop	de		; value
		pop	de
		push	hl
		jp	z,__false
		jp	nc,__true
		jp	__false


		.export __ccgteql
		.code

__ccgteql:
		call	__cmplws
		pop	hl		; return address
		pop	de		; value
		pop	de
		push	hl
		jp	z,__true
		jp	nc,__false
		jp	__true

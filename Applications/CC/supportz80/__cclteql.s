		.export __cclteql
		.code

__cclteql:
		call	__cmplws
		pop	hl		; return address
		pop	de		; value
		pop	de
		push	hl
		jp	nc,__true
		jp	__false


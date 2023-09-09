		.export __ccgtl
		.code

__ccgtl:
		call	__cmplws
		pop	hl		; return address
		pop	de		; value
		pop	de
		push	hl
		jp	nc,__false
		jp	__true

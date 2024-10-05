		.export __pushl
		.setcpu 8080
		.code

__pushl:
	xchg
	lhld	__hireg
	xthl			; swap return address with high byte
	push	d		; push low byte
	pchl			; and back



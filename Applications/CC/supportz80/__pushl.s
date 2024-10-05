		.export __pushl
		.code

__pushl:
		ex	de,hl
		ld	hl,(__hireg)
		ex	(sp),hl		; swap return address with high byte
		push	de		; push low byte
		jp	(hl)		; and back

;
;	A very very crude bootstrap from CP/M
;
	.area _MAIN(ABS)

	.org 0x0100

start:
	di
	ld	hl,#tail+0x7E00
	ld	de,#0xFFFF
	ld	bc,#0x7E00
	lddr
	jp	0x0500
tail:

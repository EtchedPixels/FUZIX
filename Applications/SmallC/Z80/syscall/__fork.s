	.code

	.export __fork

__fork:
	ld hl, 32
	jp __syscall

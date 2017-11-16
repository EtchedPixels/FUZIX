	.code

	.export __sendto

__sendto:
	ld hl, 96
	jp __syscall

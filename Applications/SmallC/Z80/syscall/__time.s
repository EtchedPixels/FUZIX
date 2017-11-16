	.code

	.export __time

__time:
	ld hl, 27
	jp __syscall

	.code

	.export __stat

__stat:
	ld hl, 15
	jp __syscall

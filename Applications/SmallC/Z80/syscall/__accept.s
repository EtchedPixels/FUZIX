	.code

	.export __accept

__accept:
	ld hl, 94
	jp __syscall

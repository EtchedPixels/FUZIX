	.code

	.export __pause

__pause:
	ld hl, 37
	jp __syscall

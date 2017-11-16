	.code

	.export __nosys82

__nosys82:
	ld hl, 82
	jp __syscall

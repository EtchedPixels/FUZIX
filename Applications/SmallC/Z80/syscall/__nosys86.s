	.code

	.export __nosys86

__nosys86:
	ld hl, 86
	jp __syscall

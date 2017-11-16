	.code

	.export __nosys84

__nosys84:
	ld hl, 84
	jp __syscall

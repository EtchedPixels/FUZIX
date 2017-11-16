	.code

	.export __nosys81

__nosys81:
	ld hl, 81
	jp __syscall

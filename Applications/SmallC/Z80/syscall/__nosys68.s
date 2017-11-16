	.code

	.export __nosys68

__nosys68:
	ld hl, 68
	jp __syscall

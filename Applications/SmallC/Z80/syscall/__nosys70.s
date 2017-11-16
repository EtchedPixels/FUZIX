	.code

	.export __nosys70

__nosys70:
	ld hl, 70
	jp __syscall

	.code

	.export __nosys85

__nosys85:
	ld hl, 85
	jp __syscall

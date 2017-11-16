	.code

	.export __nosys89

__nosys89:
	ld hl, 89
	jp __syscall

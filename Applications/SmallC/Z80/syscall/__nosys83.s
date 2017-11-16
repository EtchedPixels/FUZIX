	.code

	.export __nosys83

__nosys83:
	ld hl, 83
	jp __syscall

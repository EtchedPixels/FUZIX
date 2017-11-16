	.code

	.export _chmod

_chmod:
	ld hl, 13
	jp __syscall

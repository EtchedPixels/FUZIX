	.code

	.export _getppid

_getppid:
	ld hl, 19
	jp __syscall

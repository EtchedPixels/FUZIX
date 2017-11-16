	.code

	.export _getrlimit

_getrlimit:
	ld hl, 75
	jp __syscall

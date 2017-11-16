	.code

	.export _getpid

_getpid:
	ld hl, 18
	jp __syscall

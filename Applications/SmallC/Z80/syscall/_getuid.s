	.code

	.export _getuid

_getuid:
	ld hl, 20
	jp __syscall

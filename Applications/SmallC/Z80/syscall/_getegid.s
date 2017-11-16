	.code

	.export _getegid

_getegid:
	ld hl, 45
	jp __syscall

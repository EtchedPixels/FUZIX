	.code

	.export _getgid

_getgid:
	ld hl, 41
	jp __syscall

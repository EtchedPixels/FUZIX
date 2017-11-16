	.code

	.export _setsid

_setsid:
	ld hl, 78
	jp __syscall

	.code

	.export _getsid

_getsid:
	ld hl, 79
	jp __syscall

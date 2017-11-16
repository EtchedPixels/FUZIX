	.code

	.export _setpgid

_setpgid:
	ld hl, 77
	jp __syscall

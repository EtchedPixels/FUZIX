	.code

	.export _setgid

_setgid:
	ld hl, 26
	jp __syscall

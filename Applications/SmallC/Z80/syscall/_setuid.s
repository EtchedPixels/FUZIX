	.code

	.export _setuid

_setuid:
	ld hl, 25
	jp __syscall

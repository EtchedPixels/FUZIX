	.code

	.export _setgroups

_setgroups:
	ld hl, 73
	jp __syscall

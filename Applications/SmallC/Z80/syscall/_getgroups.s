	.code

	.export _getgroups

_getgroups:
	ld hl, 74
	jp __syscall

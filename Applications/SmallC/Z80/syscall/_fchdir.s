	.code

	.export _fchdir

_fchdir:
	ld hl, 48
	jp __syscall

	.code

	.export _rmdir

_rmdir:
	ld hl, 52
	jp __syscall

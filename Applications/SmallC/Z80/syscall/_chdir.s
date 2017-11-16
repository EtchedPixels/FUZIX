	.code

	.export _chdir

_chdir:
	ld hl, 10
	jp __syscall

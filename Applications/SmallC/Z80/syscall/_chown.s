	.code

	.export _chown

_chown:
	ld hl, 14
	jp __syscall

	.code

	.export _unlink

_unlink:
	ld hl, 6
	jp __syscall

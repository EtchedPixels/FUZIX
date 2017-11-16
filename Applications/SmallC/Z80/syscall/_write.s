	.code

	.export _write

_write:
	ld hl, 8
	jp __syscall

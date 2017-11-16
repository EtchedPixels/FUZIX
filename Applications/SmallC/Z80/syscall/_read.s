	.code

	.export _read

_read:
	ld hl, 7
	jp __syscall

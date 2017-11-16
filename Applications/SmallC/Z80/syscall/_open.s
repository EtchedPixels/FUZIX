	.code

	.export _open

_open:
	ld hl, 1
	jp __syscall

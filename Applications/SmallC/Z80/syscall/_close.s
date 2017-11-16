	.code

	.export _close

_close:
	ld hl, 2
	jp __syscall

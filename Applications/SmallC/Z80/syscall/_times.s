	.code

	.export _times

_times:
	ld hl, 42
	jp __syscall

	.code

	.export _nice

_nice:
	ld hl, 58
	jp __syscall

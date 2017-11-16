	.code

	.export _yield

_yield:
	ld hl, 62
	jp __syscall

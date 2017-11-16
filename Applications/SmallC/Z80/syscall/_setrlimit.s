	.code

	.export _setrlimit

_setrlimit:
	ld hl, 76
	jp __syscall

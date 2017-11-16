	.code

	.export _kill

_kill:
	ld hl, 39
	jp __syscall

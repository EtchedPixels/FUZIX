	.code

	.export _pipe

_pipe:
	ld hl, 40
	jp __syscall

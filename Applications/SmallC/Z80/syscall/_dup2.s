	.code

	.export _dup2

_dup2:
	ld hl, 36
	jp __syscall

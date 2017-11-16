	.code

	.export _execve

_execve:
	ld hl, 23
	jp __syscall

	.code

	.export _waitpid

_waitpid:
	ld hl, 55
	jp __syscall

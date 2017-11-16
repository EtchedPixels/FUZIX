	.code

	.export _umask

_umask:
	ld hl, 21
	jp __syscall

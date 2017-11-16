	.code

	.export _fcntl

_fcntl:
	ld hl, 47
	jp __syscall

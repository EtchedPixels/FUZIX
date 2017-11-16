	.code

	.export _connect

_connect:
	ld hl, 93
	jp __syscall

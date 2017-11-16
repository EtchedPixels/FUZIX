	.code

	.export _listen

_listen:
	ld hl, 91
	jp __syscall

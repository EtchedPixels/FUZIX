	.code

	.export _signal

_signal:
	ld hl, 35
	jp __syscall

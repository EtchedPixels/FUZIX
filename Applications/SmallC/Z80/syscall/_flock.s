	.code

	.export _flock

_flock:
	ld hl, 60
	jp __syscall

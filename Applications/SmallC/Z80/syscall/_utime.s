	.code

	.export _utime

_utime:
	ld hl, 43
	jp __syscall

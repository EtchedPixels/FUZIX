	.code

	.export _ioctl

_ioctl:
	ld hl, 29
	jp __syscall

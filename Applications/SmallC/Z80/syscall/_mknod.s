	.code

	.export _mknod

_mknod:
	ld hl, 4
	jp __syscall

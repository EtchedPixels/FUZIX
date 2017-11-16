	.code

	.export _chroot

_chroot:
	ld hl, 46
	jp __syscall

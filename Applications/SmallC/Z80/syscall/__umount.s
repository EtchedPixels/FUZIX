	.code

	.export __umount

__umount:
	ld hl, 34
	jp __syscall

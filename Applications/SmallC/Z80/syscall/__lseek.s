	.code

	.export __lseek

__lseek:
	ld hl, 9
	jp __syscall

	.code

	.export __uname

__uname:
	ld hl, 54
	jp __syscall

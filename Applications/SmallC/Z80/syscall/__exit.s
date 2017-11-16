	.code

	.export __exit

__exit:
	ld hl, 0
	jp __syscall

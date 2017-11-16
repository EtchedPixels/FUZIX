	.code

	.export __nosys87

__nosys87:
	ld hl, 87
	jp __syscall

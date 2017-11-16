	.code

	.export _rename

_rename:
	ld hl, 3
	jp __syscall

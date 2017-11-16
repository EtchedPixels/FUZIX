	.code

	.export _sync

_sync:
	ld hl, 11
	jp __syscall

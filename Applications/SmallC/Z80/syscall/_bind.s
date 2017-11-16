	.code

	.export _bind

_bind:
	ld hl, 92
	jp __syscall

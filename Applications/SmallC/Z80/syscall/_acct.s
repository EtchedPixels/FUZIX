	.code

	.export _acct

_acct:
	ld hl, 63
	jp __syscall

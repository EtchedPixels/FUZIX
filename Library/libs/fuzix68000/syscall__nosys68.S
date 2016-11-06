	.text

	.globl _nosys68

_nosys68:
.mri 1
	move.w #68,d0
	trap #14
	ext.l d1
	bne _error
	rts
_error:
	move.l d1,errno
	rts

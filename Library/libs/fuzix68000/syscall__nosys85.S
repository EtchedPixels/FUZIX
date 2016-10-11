	.text

	.globl _nosys85

_nosys85:
.mri 1
	move.w #85,d0
	trap #14
	ext.l d1
	bne _error
	rts
_error:
	move.l d1,errno
	rts

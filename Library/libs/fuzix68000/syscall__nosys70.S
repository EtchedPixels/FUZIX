	.text

	.globl _nosys70

_nosys70:
.mri 1
	move.w #70,d0
	trap #14
	ext.l d1
	bne _error
	rts
_error:
	move.l d1,errno
	rts

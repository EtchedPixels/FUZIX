	.text

	.globl _nosys67

_nosys67:
.mri 1
	move.w #67,d0
	trap #14
	ext.l d1
	bne _error
	rts
_error:
	move.l d1,errno
	rts

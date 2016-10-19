	.text

	.globl _fork

_fork:
.mri 1
	movem.l d2-d7/a2-a6,-(sp)
	move.w #32,d0
	trap #14
	ext.l d1
	bne _error
	movem.l (sp)+,d2-d7/a2-a6
	rts
_error:
	move.l d1,errno
	movem.l (sp)+,d2-d7/a2-a6
	rts

	.text

	.globl _fork

_fork:
.mri 1
	movem.l d2-d7/a2-a4/a6,-(sp)
	move.l 44(sp),d1
	move.w #32,d0
	trap #14
	movem.l (sp)+,d2-d7/a2-a4/a6
	ext.l d1
	bne _error
	rts
_error:
	move.l d1,errno
	rts

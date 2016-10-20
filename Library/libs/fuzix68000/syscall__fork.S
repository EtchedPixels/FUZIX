	.text

	.globl _fork

_fork:
.mri 1
	move.l 4(sp),d1
	movem.l d0-d7/a2-a6,-(sp)
	move.w #32,d0
	trap #14
	ext.l d1
	bne _error
	addq #8,sp
	movem.l (sp)+,d2-d7/a2-a6
	rts
_error:
	move.l d1,errno
	addq #8,sp
	movem.l (sp)+,d2-d7/a2-a6
	rts

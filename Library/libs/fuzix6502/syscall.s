	.export __syscall
	.import _errno
	.import __syscall_hook

.proc __syscall
	jsr	__syscall_hook
	bne	error
	tya		; x,a form the return code
	rts
error:
	sta	_errno
	ldx	#0
	stx	_errno+1
	dex	;	Return $FFFF (-1)
	txa
	rts

.endproc

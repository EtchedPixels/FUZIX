	.export __syscall
	.import _errno

.proc __syscall
	jsr	callfe
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
callfe:	jmp	($fe)

.endproc

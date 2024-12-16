;
;	Initial 6800 crt0.
;

	.code

	.export _environ
	.export head

head:
	.word	$80A8
	.byte	2		;	6800 series
	.byte	0		;	Needs only 6800 features
	.byte   1		;	Load at 0x100
	.byte	0		;	No hints
	.word	__data-0x0100	;	Code
	.word	__data_size	;	Data
	.word	__bss_size
	.byte	<start		;	Offset to execute from
	.byte	0		;	No size hint
	.byte	0		;	No stack hint
	.byte	0		;	No hint bits

	.word   __sighandler	;	TODO: signals

;
;	This function is called when we need to deliver a signal. We can't
;	just blindly stack stuff as we can on big machines because we have
;	non-reentrancy issues in the compiler temporary and regvar usage
;
;	On entry
;	B = signal number
;	X = undefined
;
;	Return address is the correct route back to the kernel. Above it is
;	a copy of the vector and an RTI frame.
;
__sighandler:
	; Save compiler temporaries and dp register variables
	ldaa @tmp
	psha
	ldaa @tmp+1
	psha
	ldaa @tmp1
	psha
	ldaa @tmp1+1
	psha
	ldaa @tmp2
	psha
	ldaa @tmp2+1
	psha
	ldaa @hireg
	psha
	ldaa @hireg+1
	psha
	ldaa @tmp3
	psha
	ldaa @tmp3+1
	psha
	ldaa @tmp4
	psha
	ldaa @tmp4+1
	psha
	ldaa @tmp5
	psha
	ldaa @tmp5+1
	psha
	pshb		; signal number
	clra
	psha		; extended to 16bits

	;
	;	Fishing time. Our vector is up the stack above all the
	;	stuff we pushed

	tsx
	ldx 18,x
	jsr ,x
	; ABI has callee removing argument
	; Restore the C direct page
	pula
	staa @tmp5+1
	pula
	staa @tmp5
	pula
	staa @tmp4+1
	pula
	staa @tmp4
	pula
	staa @tmp3+1
	pula
	staa @tmp3
	pula
	staa @hireg+1
	pula
	staa @hireg
	pula
	staa @tmp2+1
	pula
	staa @tmp2
	pula
	staa @tmp1+1
	pula
	staa @tmp1
	pula
	staa @tmp+1
	pula
	staa @tmp
	rts

start:
	clrb
	stab	@zero
	stab	@zero+1
	incb
	stab	@one+1
	tsx
	inx
	inx
	inx
	inx
	stx	_environ
	; Now call main
	jsr	_main
	pshb
	psha
	jmp	_exit

	.bss
_environ:
	.word	0

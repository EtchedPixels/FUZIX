;
;	Initial 6303 crt0. We need to add self relocation etc to it
;

	.setcpu 6303
	.code

	.export _environ
	.export head

head:
	.word	$80A8
	.byte	2		;	6800 series
	.byte	3		;	Needs 6803 and 6303 features
	.byte   >head		;	Load page
	.byte	0		;	No hints
	.word	__code_size
	.word	__data_size
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
;	D = signal number
;	X = undefined
;
;	Return address is the correct route back to the kernel. Above it is
;	a copy of the vector and an RTI frame.
;
__sighandler:
	; Save compiler temporaries and dp register variables
	ldx	@tmp
	pshx
	ldx	@tmp1
	pshx
	ldx	@tmp2
	pshx
	ldx	@tmp3
	pshx
	ldx	@tmp4
	pshx
	ldx	@fp
	pshx
	ldx	@reg
	pshx
	ldx	@reg+2
	pshx
	ldx	@reg+4
	pshx
	ldx	@sreg
	pshx
	; Arguments signal number and frame pointer
	pshb	; Save the signal number
	psha
	tsx
	pshx
	ldx	26,x	; vector
	jsr	,x
	; Discard arguments
	pulx
	pulx
	pulx
	; Restore temporaries
	stx	@sreg
	pulx
	stx	@reg+4
	pulx
	stx	@reg+2
	pulx
	stx	@reg
	pulx
	stx	@fp
	pulx
	stx	@tmp4
	pulx
	stx	@tmp3
	pulx
	stx	@tmp2
	pulx
	stx	@tmp
	; Back to kernel provided address
	rts

start:
	clra
	clrb
	std	@zero
	incb
	stab	@one+1
	jsr	___stdio_init_vars
	tsx
	ldab	#4
	abx
	stx	_environ
	; Now call main
	decb		; In case someone defines it vararg! (4 bytes of arg)
	jsr	_main
	pshb
	psha
	jmp	_exit

	.bss
_environ:
	.word	0

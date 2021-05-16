;
;	Initial 68HC11 crt0. We need to add self relocation etc to it
;

	.text

	.globl environ

head:
	.word	0x80A8
	.byte	2		;	6800 series
	.byte	4		;	Needs 68HC11 features
	.byte   1 		;	Load page (FIXME hardcoded)
	.byte	0		;	No hints
	.word	__code_end-0x100	;	code_size
	.word	__data_size	;	data_size
	.word	__bss_size	;	bss_size
	.byte	__start-head	;	Offset to execute from
	.byte	0		;	No size hint
	.byte	0		;	No stack hint
	.byte	0		;	No hint bits

	.word   __sighandler	;	Signals

;
;	This function is called when we need to deliver a signal. We can't
;	just blindly stack stuff as we can on big machines because we have
;	non-reentrancy issues in the compiler temporary and regvar usage
;
;	On entry
;	D = signal number
;	Y = address of signal handler
;	X = undefined
;
;	Return address is the correct route back to the kernel. Above it is
;	a copy of the vector and an RTI frame.
;
__sighandler:
	; Save our compiler temporaries and dp register variables
	
	ldx	*_.tmp
	pshx
	ldx	*_.xy
	pshx
	ldx	*_.z
	pshx
	ldx	*_.d1
	pshx
	ldx	*_.d2
	pshx
	ldx	*_.d3
	pshx
	ldx	*_.d4
	pshx
	tsx
	pshx		; stack the address base of the signal frame
			; for the caller
	jsr	,y	; call the C signal handler

	pulx		; discard signal stack info
	pulx
	stx	*_.d4
	pulx
	stx	*_.d3
	pulx
	stx	*_.d2
	pulx
	stx	*_.d1
	pulx
	stx	*_.z
	pulx
	stx	*_.xy
	pulx
	stx	*_.tmp
	rts		; to the return provided by the kernel

;
;	4(sp)+	environ
;	2(sp)	argv
;	0(sp)	argc
__start:
	jsr	__stdio_init_vars
	tsx
	ldab	#4
	abx
	stx	environ
	tsx
	ldd	2,x
	; Now call main. After the call is stacked We need
	; D = argc, 2(sp) = argv
	jsr	main
	pshb
	psha
	jmp	exit

	.comm environ,2,1

;
;	Initial 6303 crt0. We need to add self relocation etc to it
;

	.setcpu 6303
	.code

	.export _environ

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

__sighandler:
	rts

start:
	clra
	clrb
	std	@zero
	incb
	stab	@one+1
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

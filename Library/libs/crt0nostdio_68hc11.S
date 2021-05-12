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

	.word   __sighandler	;	TODO: signals

__sighandler:
	rts

;	4(sp)+	environ
;	2(sp)	argv
;	0(sp)	argc
__start:
	tsx
	ldab	#4
	abx
	stx	environ
	tsx
	ldd	2,x
	; Now call main. After the call is staked We need
	; D = argc, 2(sp) = argv
	jsr	main
	pshb
	psha
	jmp	exit

	.comm environ,2,1

;
;	Relocating 68HC11 crt0
;

	.text
	.mode mshort

	.globl environ
	.globl __sysvector

head:
__sysvector:
	.word	0x80A8
	.byte	2		;	6800 series
	.byte	4		;	Needs 68HC11 features
	.byte   0 		;	Load page (relocatable)
	.byte	0		;	No hints
	.word	__code_end-0x100	;	code_size
	.word	__data_size	;	data_size
	.word	__bss_size	;	bss_size
	.byte	__start-head	;	Offset to execute from
	.byte	0		;	No size hint
	.byte	0		;	No stack hint
	.byte	0		;	DP size (filled in later)

	.word   __sighandler	;	Signals
	.word	0		;	Relocation pointer to be

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


relnext:
	iny
reloc:	ldab	,y
	beq	done
	clr	,y
	cmpb	#0xff
	bne	relone
	; 0xFF means move on 0xFE bytes and don't do a relocation but
	; read a further relocation byte
	decb
	abx
	bra	relnext
relone:
	; 0x01-0xFE means move on that many bytes and then relocate that
	; byte
	abx
	tab
	addb	,x
	stab	,x
	bra	relnext
done:
	rts

;
;	4(sp)+	environ
;	2(sp)	argv
;	0(sp)	argc
;
;	A = base of executable (high)
;	B = base of direct page for user usage
;	X/Y undefined
;
;	Some trickery is required to do this without using DP and entirely
;	relocatably on an HC11.
;	
__start:
	psha	; save high byte of base address for second relocation set 
	pshb	; save DP relocation for the moment
	psha	; save high byte of base address again 
	clrb	; D is now the base address
	xgdx	; X is now the base address so we can use it for PC relative
	ldaa	18,x	; relocation table base into Y
	; We have to relocate this ourselves
	pulb	; get high byte of base address back
	aba
	ldab	19,x	; low byte does not need relocating
	xgdy	; Y now holds the relocation table pointer
	pula	; Recover the DP relocation value
	pshx	; save base
	bsr	reloc
	pulx	; restore base
	pula	; recover high byte of base copy (for second reloc run)
	iny	; Past the end marker
	bsr	reloc
	ldd	#__common_end	; by now relocated
	jsr	brk	; fix the brk base if it changed for relocations

	;
	;	And into the main flow
	;

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
